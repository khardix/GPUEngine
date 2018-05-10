/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Visualisation technique for displaying models (implementation).
 */

#include <stdexcept>
#include <string>

#include <geGL/Buffer.h>

#include "scene_util.hh"
#include "visualization.hh"


/// @brief Hardcoded source for vertex shader of the uniform program
const std::string UniformVisualization::VERTEX_CODE = R"vertex(
#version 430 core

layout (location = 0) in vec3 model_vertex;

void main() {
    gl_Position = vec4(model_vertex, 1.0);
}
)vertex";

/// @brief Hardcoded source for geometry shader of the uniform program
const std::string UniformVisualization::GEOMETRY_CODE = R"geometry(
#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos = vec3(0.0, 0.0, 100.0);

out GS_OUT {
    flat vec3 N;
    flat vec3 L;
    flat vec3 V;
} gs_out;

void main() {
    // calculate the normal
    vec3 side_a = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 side_b = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(side_a, side_b));

    // generate transformed vertices
    int i = 0;
    vec4 P = vec4(1.0);
    for (i = 0; i < gl_in.length(); ++i) {
        P = view * model * gl_in[i].gl_Position;
        gs_out.N = mat3(view * model) * normal;
        gs_out.L = light_pos - P.xyz;
        gs_out.V = -P.xyz;

        gl_Position = projection * P; EmitVertex();
    }
    EndPrimitive();
}
)geometry";

/// @brief Hardcoded source for fragment shader of the uniform program
const std::string UniformVisualization::FRAGMENT_CODE = R"fragment(
#version 430 core

uniform vec3 ambient = vec3(0.1, 0.1, 0.1);
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.3);
uniform float specular_power = 32.0;

in GS_OUT {
    flat vec3 N;
    flat vec3 L;
    flat vec3 V;
} fs_in;

out vec3 color;

void main() {
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(fs_in.L);
    vec3 V = normalize(fs_in.V);

    vec3 R = reflect(-L, N);
    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
    vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_albedo;

    color = diffuse + specular + ambient;
}
)fragment";


/** Compiles hard-coded sources into usable OpenGL program.
 * @throws std::runtime_error Compilation/linking failed.
 */
UniformVisualization::UniformVisualization()
{
    using ge::gl::Program;
    using ge::gl::Shader;

    m_program = std::make_unique<Program>(
        std::make_shared<Shader>(GL_VERTEX_SHADER, VERTEX_CODE),
        std::make_shared<Shader>(GL_GEOMETRY_SHADER, GEOMETRY_CODE),
        std::make_shared<Shader>(GL_FRAGMENT_SHADER, FRAGMENT_CODE));

    if (!static_cast<bool>(m_program->getLinkStatus())) {
        throw std::runtime_error(
            "Uniform shading program compilation/linking failed");
    }
}

/** @note Not every attribute of a mesh is utilized by the underlying program.
 * See return value description for more.
 * @param[in] semantic Input semantic.
 * @returns Attribute binding (≥ 0) for an attribute with the input semantic.
 * If the program does not utilize an attribute with such semantic, returns -1,
 */
GLint UniformVisualization::semantic_binding(
    ge::sg::AttributeDescriptor::Semantic semantic) noexcept
{
    using Semantic = ge::sg::AttributeDescriptor::Semantic;

    switch (semantic) {
        default:
            return -1;

        case Semantic::position:
            return 0;
    }
}

/** Create VertexArray object from a single Mesh. All used attributes are
 * converted as they are.
 * @param[in] mesh The source mesh.
 * @returns Unique pointer to new filled VertexArray.
 */
std::unique_ptr<ge::gl::VertexArray> UniformVisualization::convert(
    const ge::sg::Mesh &mesh)
{
    using Semantic = ge::sg::AttributeDescriptor::Semantic;

    auto result = std::make_unique<ge::gl::VertexArray>();

    for (const auto &descriptor : mesh.attributes) {
        auto buffer = std::make_unique<ge::gl::Buffer>(
            static_cast<const GLsizeiptr>(descriptor->size),
            static_cast<const GLvoid *>(descriptor->data.get()));

        if (descriptor->semantic == Semantic::indices) {
            result->addElementBuffer(std::move(buffer));
            continue;
        }
        auto binding = semantic_binding(descriptor->semantic);
        if (binding < 0) {
            continue;
        }

        result->addAttrib(
            std::move(buffer),
            static_cast<const GLuint>(binding),
            static_cast<const GLint>(descriptor->numComponents),
            util::glsg::translate(descriptor->type),
            static_cast<const GLsizei>(descriptor->stride),
            static_cast<const GLintptr>(descriptor->offset));
    }

    return std::move(result);
}

/** Iterates over each model and mesh in the scene, drawing them in order. Model
 * transformations are respected.
 * @post The internal program is in use.
 * @post Last VAO of the scene is bound.
 * @param[in] context OpenGL context to use for drawing. Should be active.
 * @param[in] scene The scene to draw.
 * @throws std::runtime_error Mesh has no attribute with Semantic::indices.
 */
void UniformVisualization::draw(
    ge::gl::Context &context, std::shared_ptr<ge::sg::Scene> scene)
{
    using MeshContainer = util::SceneWalker::iterator::MeshContainer &;
    using Transformation = glm::mat4;
    using Semantic = ge::sg::AttributeDescriptor::Semantic;

    m_program->use();

    for (const auto &model : util::SceneWalker(scene)) {
        const auto &meshes = std::get<MeshContainer>(model);
        const auto &transformation = std::get<Transformation>(model);

        m_program->setMatrix4fv("model", glm::value_ptr(transformation));

        for (const auto &mesh : meshes) {
            const auto &indices = mesh->getAttribute(Semantic::indices);
            if (!indices) {
                throw std::runtime_error("Encountered mesh with no indices");
            }

            auto &vao = m_vao_cache[mesh.get()];
            /* if (!vao) { */
            vao = convert(*mesh);
            /* } */

            const auto &mode = util::glsg::translate(mesh->primitive);
            const auto &count = static_cast<GLsizei>(mesh->count);
            const auto &type = util::glsg::translate(indices->type);

            vao->bind();
            context.glDrawElements(mode, count, type, nullptr);
        }
    }
}
