/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Visualisation technique for displaying models (implementation).
 */

#include <stdexcept>
#include <string>

#include "scene.hh"
#include "visualization.hh"


/// @brief Hardcoded source for vertex shader of the uniform program
static const auto UNIFORM_VERTEX_CODE = std::string{R"vertex(
#version 430 core

layout (location = 0) in vec3 model_vertex;

void main() {
    gl_Position = vec4(model_vertex, 1.0);
}
)vertex"};

/// @brief Hardcoded source for geometry shader of the uniform program
static const auto UNIFORM_GEOMETRY_CODE = std::string{R"geometry(
#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 modelview;
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
        P = modelview * gl_in[i].gl_Position;
        gs_out.N = mat3(modelview) * normal;
        gs_out.L = light_pos - P.xyz;
        gs_out.V = -P.xyz;

        gl_Position = projection * P; EmitVertex();
    }
    EndPrimitive();
}
)geometry"};

/// @brief Hardcoded source for fragment shader of the uniform program
static const auto UNIFORM_FRAGMENT_CODE = std::string{R"fragment(
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
)fragment"};


/** Compiles hard-coded sources into usable OpenGL program.
 * @returns Compiled and linked program.
 * @throws std::runtime_error Compilation/linking failed.
 */
std::unique_ptr<ge::gl::Program> make_uniform_program()
{
    using ge::gl::Program;
    using ge::gl::Shader;

    auto result = std::make_unique<Program>(
        std::make_shared<Shader>(GL_VERTEX_SHADER, UNIFORM_VERTEX_CODE),
        std::make_shared<Shader>(GL_GEOMETRY_SHADER, UNIFORM_GEOMETRY_CODE),
        std::make_shared<Shader>(GL_FRAGMENT_SHADER, UNIFORM_FRAGMENT_CODE));

    if (!static_cast<bool>(result->getLinkStatus())) {
        throw std::runtime_error(
            "Uniform shading program compilation/linking failed");
    }

    return std::move(result);
}
