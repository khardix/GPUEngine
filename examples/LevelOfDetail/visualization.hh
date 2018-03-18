/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Visualization technique for displaying models.
 */

#pragma once

#include <memory>
#include <unordered_map>

#include <glm/mat4x4.hpp>

#include <geGL/OpenGLContext.h>
#include <geGL/Program.h>
#include <geGL/VertexArray.h>
#include <geSG/Mesh.h>
#include <geSG/Scene.h>
#include <geUtil/MatrixStack.h>

/// @brief Cached VAOs storage for rendered meshes
using vao_map = std::unordered_map<
    const ge::sg::Mesh *const,
    std::unique_ptr<ge::gl::VertexArray>>;

/// @brief Rendering program using uniform color, material and flat normals.
std::unique_ptr<ge::gl::Program> make_uniform_program();

/// @brief Render a scene.
void render(
    const ge::sg::Scene &  scene,
    const ge::gl::Program &program,
    ge::util::MatrixStack &matrix_stack,
    vao_map &              vao_cache);
