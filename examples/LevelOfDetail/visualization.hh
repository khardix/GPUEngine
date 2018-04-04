/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Visualization technique for displaying models.
 */

#pragma once

#include <memory>
#include <unordered_map>

#include <geGL/Program.h>
#include <geGL/VertexArray.h>
#include <geSG/Mesh.h>

/// @brief Cached VAOs storage for rendered meshes
using vao_map = std::unordered_map<
    const ge::sg::Mesh *const,
    std::unique_ptr<ge::gl::VertexArray>>;

/// @brief Rendering program using uniform color, material and flat normals.
std::unique_ptr<ge::gl::Program> make_uniform_program();

namespace util {
}  // namespace util

// Inline and template members {{{
// Inline and template members {{{
