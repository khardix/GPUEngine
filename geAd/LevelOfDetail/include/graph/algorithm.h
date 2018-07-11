#ifndef ALGORITHM_H_SMHGX2NT
#define ALGORITHM_H_SMHGX2NT
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Generic graph algorithms.
 *
 * The purpose of this file is to break circular dependency between Node.h and
 * Edge.h, and provide prepared graph traversal and analysis algorithms.
 */

#include <array>
#include <deque>
#include <stdexcept>

#include "Edge.h"
#include "Node.h"
#include "Triangle.h"

namespace lod {
namespace graph {
/// @brief An algorithm cannot process the passed topology.
struct algorithm_failure : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// @brief List all edges opposite to center node.
std::deque<DirectedEdge::pointer_type> opposite_edges(const Node &center);

/// @brief List all adjacent nodes in edge order.
std::deque<Node::const_weak_type> adjacent_nodes(
    const std::deque<DirectedEdge::pointer_type> &edge_ring);
/** @overload
 * @param[in] center The center node.
 * @return Container of nodes adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
inline std::deque<Node::const_weak_type> adjacent_nodes(const Node &center)
{
    return adjacent_nodes(opposite_edges(center));
}

/// @brief List all adjacent triangles in edge order.
std::deque<Triangle> adjacent_triangles(
    const std::deque<DirectedEdge::pointer_type> &edge_ring);
/** @overload
 * @param[in] center The center node.
 * @return Container of triangles adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
inline std::deque<Triangle> adjacent_triangles(const Node &center)
{
    return adjacent_triangles(opposite_edges(center));
}

/// @brief List all emanating edges.
std::deque<DirectedEdge::pointer_type> emanating_edges(
    const std::deque<DirectedEdge::pointer_type> &edge_ring);
/** @overload
 * @param[in] center The center node.
 * @return Container of edges emanating from center.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
inline std::deque<DirectedEdge::pointer_type> emanating_edges(
    const Node &center)
{
    return emanating_edges(opposite_edges(center));
}
}  // namespace graph
}  // namespace lod

#endif /* end of include guard: ALGORITHM_H_SMHGX2NT */
