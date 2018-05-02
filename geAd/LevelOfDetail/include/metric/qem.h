#ifndef QEM_H_1O2EFAO6
#define QEM_H_1O2EFAO6
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Quadric Error Metric (QEM).
 */

#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "../graph/Edge.h"
#include "../graph/Mesh.h"
#include "../graph/Node.h"
#include "../graph/Triangle.h"
#include "../protocol.h"

namespace lod {
namespace metric {
class QEM {
public:
    using Element = graph::Mesh::EdgeSet::value_type;
    using Result = operation::VertexPlacement<Element, float>;

    using cache_type
        = std::unordered_map<graph::Node::position_type, glm::mat4>;

    explicit QEM() = default;
    explicit QEM(cache_type cache);

    /// @brief Calculate the cost of removing an edge.
    Result operator()(const Element &edge) const;

    /// @brief Calculate planar vector for a triangle.
    static const glm::vec4 plane(const graph::Triangle &triangle);
    /// @brief Calculate quadric for specified node.
    const glm::mat4 &quadric(const graph::Node &node) const;
    /// @brief Calculate optimal position of a new node from quadric.
    static glm::vec3 position(const glm::mat4 &quadric);
    /// @brief Calculate the margin of error.
    static Result::cost_type error(const glm::mat4 &quadric);
    static Result::cost_type error(
        const glm::mat4 &quadric, const glm::vec3 &position);

private:
    /// Cached evaluations of existing vertices.
    mutable cache_type m_vertex_cache = {};
};
}  // namespace metric
}  // namespace lod


// Inline and template members
inline lod::metric::QEM::QEM(cache_type cache)
    : m_vertex_cache(std::move(cache))
{
}

/** @param[in] quadric The quadric to calculate the error from.
 * @returns The margin of error for a new node with optimal placement.
 */
inline lod::metric::QEM::Result::cost_type lod::metric::QEM::error(
    const glm::mat4 &quadric)
{
    return error(quadric, position(quadric));
}

#endif /* end of include guard: QEM_H_1O2EFAO6 */
