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

uniform mat4 modelview;
uniform mat4 projection;

layout (location = 0) in vec3 model_vertex;

void main() {
    gl_Position = projection * modelview * vec4(model_vertex, 1.0);
}
)vertex"};

/// @brief Hardcoded source for geometry shader of the uniform program
static const auto UNIFORM_GEOMETRY_CODE = std::string{R"geometry(
#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main() {
    int i = 0;

    for (i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position; EmitVertex();
    }
    EndPrimitive();
}
)geometry"};

/// @brief Hardcoded source for fragment shader of the uniform program
static const auto UNIFORM_FRAGMENT_CODE = std::string{R"fragment(
#version 430 core

uniform vec3 base_color = vec3(0.5, 0.2, 0.7);

out vec3 color;

void main() {
    color = base_color;
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
