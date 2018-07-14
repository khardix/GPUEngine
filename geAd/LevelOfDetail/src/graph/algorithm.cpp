/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Generic graph algorithms (implementation).
 */

#include <algorithm>
#include <cassert>
#include <exception>
#include <iterator>

#include <graph/algorithm.h>

namespace lod {
namespace graph {
/** Attempts to list all edges in adjacent triangles opposite to center node.
 * @todo Currently cannot deal with non-manifold vertices.
 * @see Campagna et al.: Directed Edges - A Scalable Representation for Triangle
 * Meshes, 1998.
 * @param[in] center The center node.
 * @return Container of edges in order.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<DirectedEdge::pointer_type> opposite_edges(const Node &center) try {
    if (center.edge().expired()) {
        throw std::runtime_error("Sole node!");
    }
    auto result = std::deque<DirectedEdge::pointer_type>{};

    const auto next = [](const DirectedEdge::const_pointer_type &edge) {
        auto weak_neighbour
            = nonstd::get<DirectedEdge::weak_type>(edge->next()->neighbour());
        if (auto neigh = weak_neighbour.lock()) {
            return neigh->next();
        }
        return DirectedEdge::pointer_type(nullptr);
    };
    const auto prev = [](const DirectedEdge::const_pointer_type &edge) {
        auto weak_neighbour = nonstd::get<DirectedEdge::weak_type>(
            edge->previous().lock()->neighbour());
        if (auto neigh = weak_neighbour.lock()) {
            return neigh->previous().lock();
        }
        return DirectedEdge::pointer_type(nullptr);
    };

    // walk until a mesh boundary or full circle
    auto center_edge = center.edge().lock();
    auto pivot = center_edge->target().lock();
    for (auto edge = center_edge->next(); edge; edge = next(edge)) {
        result.push_back(edge);
        if (edge->target().lock() == pivot) {  // full circle
            return result;
        }
    }

    // stopped at boundary; check if we need to walk backwards as well
    if (center_edge->boundary()) {
        return result;
    }

    pivot = result.back()->target().lock();
    auto center_back
        = nonstd::get<DirectedEdge::weak_type>(center_edge->neighbour()).lock();
    for (auto edge = center_back->previous().lock(); edge; edge = prev(edge)) {
        if (edge->target().lock() == pivot) {
            return result;
        }
        result.push_front(edge);
    }

    return result;
}
catch (const nonstd::bad_variant_access &) {
    auto message = algorithm_failure("Non-manifold edge in adjecent_nodes");
    std::throw_with_nested(message);
}

/** Attempts to list all adjacent nodes in "edge order"
 * (source Node before target Node for each edge opposite to center).
 * @todo Currently cannot deal with non-manifold vertices.
 * @see lod::graph::opposite_edges().
 * @param[in] edge_ring The ring of edges surrounding a node.
 * @return Container of nodes adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<Node::weak_type> adjacent_nodes(
    const std::deque<DirectedEdge::pointer_type> &edge_ring)
{
    auto full_circle = [](const auto &edges) -> bool {
        return (edges.front()->previous().lock()->target().lock())
            == (edges.back()->target().lock());
    };

    auto result = std::deque<Node::weak_type>{};

    std::transform(
        std::cbegin(edge_ring),
        std::cend(edge_ring),
        std::back_inserter(result),
        [](const auto &edge) { return edge->target(); });

    if (!full_circle(edge_ring)) {  // add the first node explicitly
        result.push_front(edge_ring.front()->previous().lock()->target());
    }

    return result;
}

/** Attempts to list all adjacent triangles in edge order.
 * @todo Currently cannot deal with non-manifold vertices.
 * @see lod::graph::opposite_edges().
 * @param[in] edge_ring The ring of edges surrounding a node.
 * @return Container of triangles adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<Triangle> adjacent_triangles(
    const std::deque<DirectedEdge::pointer_type> &edge_ring)
{
    auto result = std::deque<Triangle>{};

    std::transform(
        std::cbegin(edge_ring),
        std::cend(edge_ring),
        std::back_inserter(result),
        [](const auto &edge) {
            return std::static_pointer_cast<const DirectedEdge>(edge)
                ->triangle_edges();
        });
    return result;
}

/** Attempts to list all emanating edges.
 * @todo Currently cannot deal with non-manifold vertices.
 * @see lod::graph::opposite_edges().
 * @param[in] edge_ring The ring of edges surrounding a node.
 * @return Container of edges emanating from the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<DirectedEdge::pointer_type> emanating_edges(
    const std::deque<DirectedEdge::pointer_type> &edge_ring)
{
    auto result = std::deque<DirectedEdge::pointer_type>{};

    std::transform(
        std::cbegin(edge_ring),
        std::cend(edge_ring),
        std::back_inserter(result),
        [](const auto &edge) { return edge->previous().lock(); });
    return result;
}
}  // namespace graph
}  // namespace lod
