/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Visualization technique for displaying models.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <geGL/Program.h>
#include <geGL/VertexArray.h>
#include <geSG/Mesh.h>

/** @brief Visualization technique using uniform color, material and flat
 * normals.
 */
class UniformVisualization {
public:
    friend class GLView;

    /// @brief Cached VAOs storage for rendered meshes
    using vao_map = std::unordered_map<
        const ge::sg::Mesh *,
        std::unique_ptr<ge::gl::VertexArray>>;

    /// @brief Compile the underlying shader program.
    explicit UniformVisualization();

    /// @brief Set view transformation.
    void view_matrix(const glm::mat4 &matrix) noexcept;
    /// @brief Set projection transformation.
    void projection_matrix(const glm::mat4 &matrix) noexcept;

    /// @brief Draw a scene using provided context.
    void draw(ge::gl::Context &context, std::shared_ptr<ge::sg::Scene> scene);

protected:
    static const std::string VERTEX_CODE;    ///< Vertex shader source.
    static const std::string GEOMETRY_CODE;  ///< Geometry shader source.
    static const std::string FRAGMENT_CODE;  ///< Fragment shader source.

    /// @brief Convert generic semantic to appropriate attribute binding index.
    static GLint semantic_binding(
        ge::sg::AttributeDescriptor::Semantic semantic) noexcept;
    /// @brief Convert generic mesh description to OpenGL VAO.
    static std::unique_ptr<ge::gl::VertexArray> convert(
        const ge::sg::Mesh &mesh);

private:
    std::unique_ptr<ge::gl::Program> m_program = nullptr;
    mutable vao_map                  m_vao_cache = {};
};

// Inline and template members {{{
/** @param[in] matrix New view transformation matrix. */
inline void UniformVisualization::view_matrix(const glm::mat4 &matrix) noexcept
{
    m_program->setMatrix4fv("view", glm::value_ptr(matrix));
}

/** @param[in] matrix New projection transformation matrix. */
inline void UniformVisualization::projection_matrix(
    const glm::mat4 &matrix) noexcept
{
    m_program->setMatrix4fv("projection", glm::value_ptr(matrix));
}
// Inline and template members }}}
