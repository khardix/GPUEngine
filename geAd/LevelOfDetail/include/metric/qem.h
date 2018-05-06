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
/** @tparam Tag The tag type to select algorithm details.
 *  Supported values are HalfEdgeTag and FullEdgeTag.
 */
template <typename Tag>
class QEM {
public:
    using element_type = typename Tag::element_type;
    using result_type = operation::VertexPlacement<element_type, float>;

    using cache_type
        = std::unordered_map<graph::Node::position_type, glm::mat4>;

    explicit QEM() = default;
    explicit QEM(cache_type cache);

    /// @brief Calculate planar vector for a triangle.
    static const glm::vec4 plane(const graph::Triangle &triangle);
    /// @brief Calculate quadric for specified node.
    const glm::mat4 &quadric(const graph::Node &node) const;
    /// @brief Calculate optimal position of a new node from quadric.
    static glm::vec3 position(const glm::mat4 &quadric);
    /// @brief Calculate the margin of error.
    static typename result_type::cost_type error(const glm::mat4 &quadric);
    static typename result_type::cost_type error(
        const glm::mat4 &quadric, const glm::vec3 &position);

    /// @brief Calculate the cost of removing an edge.
    result_type operator()(const element_type &edge) const;

private:
    /// Cached evaluations of existing vertices.
    mutable cache_type m_vertex_cache = {};
};


// Inline and template members
template <typename Tag>
inline QEM<Tag>::QEM(cache_type cache) : m_vertex_cache(std::move(cache))
{
}

/** @param[in] quadric The quadric to calculate the error from.
 * @returns The margin of error for a new node with optimal placement.
 */
template <typename Tag>
inline typename QEM<Tag>::result_type::cost_type QEM<Tag>::error(
    const glm::mat4 &quadric)
{
    return error(quadric, position(quadric));
}

/** Calculate the collapse cost and provide optimal new node placement.
 * @param[in] edge The edge to evaluate.
 * @returns Cost and placement suggestion.
 */
template <>
inline auto QEM<operation::FullEdgeTag>::operator()(
    const element_type &edge) const -> result_type
{
    // calculate quadric of the new node: Q = Q1 + Q2
    const auto &nodes
        = std::make_pair(edge->target(), edge->previous().lock()->target());
    auto qmat = quadric(*std::get<0>(nodes)) + quadric(*std::get<1>(nodes));

    const auto pos = position(qmat);
    const auto err = error(qmat, pos);

    m_vertex_cache.emplace(pos, std::move(qmat));
    return result_type(edge, err, pos);
}

template <>
inline auto QEM<operation::HalfEdgeTag>::operator()(
    const element_type &edge) const -> result_type
{
    const auto &nodes
        = std::make_pair(edge->target(), edge->previous().lock()->target());
    auto qmat = quadric(*std::get<0>(nodes)) + quadric(*std::get<1>(nodes));

    // calculate error of collapse to the target node
    const auto err = error(qmat, edge->target()->position);
    return result_type(edge, err, edge->target()->position);
}

extern template class QEM<operation::FullEdgeTag>;
extern template class QEM<operation::HalfEdgeTag>;

}  // namespace metric
}  // namespace lod
#endif /* end of include guard: QEM_H_1O2EFAO6 */
