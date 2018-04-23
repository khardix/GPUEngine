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
#include "../graph/Node.h"
#include "../graph/Triangle.h"
#include "../protocol.h"

namespace lod {
namespace metric {
class QEM {
public:
    using Element = graph::UndirectedEdge;
    using Result = cost::VertexPlacement;

    using cache_type
        = std::unordered_map<graph::Node::position_type, glm::mat4>;

    explicit QEM() = default;
    explicit QEM(cache_type cache);

    /// @brief Calculate the cost of removing an edge.
    Result operator()(const Element &edge) const;

protected:
    /// @brief Calculate planar vector for a triangle.
    static const glm::vec4 plane(const graph::Triangle &triangle);
    /// @brief Calculate quadric for specified node.
    const glm::mat4 &quadric(const graph::Node &node) const;

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

#endif /* end of include guard: QEM_H_1O2EFAO6 */
