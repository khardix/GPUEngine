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
std::deque<const DirectedEdge *> opposite_edges(const Node &center) try {
    if (center.edge == nullptr) {
        throw std::runtime_error("Sole node!");
    }
    auto result = std::deque<const DirectedEdge *>{};

    const auto next = [](const DirectedEdge *const edge) -> DirectedEdge * {
        auto neigh = nonstd::get<DirectedEdge *>(edge->next()->neighbour);
        return neigh == nullptr ? nullptr : neigh->next();
    };
    const auto prev = [](const DirectedEdge *const edge) -> DirectedEdge * {
        auto neigh = nonstd::get<DirectedEdge *>(edge->previous->neighbour);
        return neigh == nullptr ? nullptr : neigh->previous;
    };

    // walk until a mesh boundary or full circle
    auto pivot = center.edge->target;
    for (auto edge = center.edge->next(); edge != nullptr; edge = next(edge)) {
        result.push_back(edge);
        if (edge->target == pivot) {  // full circle
            return result;
        }
    }

    // stopped at boundary; check if we need to walk backwards as well
    if (center.edge->boundary()) {
        return result;
    }

    pivot = result.back()->target;
    auto center_back = nonstd::get<DirectedEdge *>(center.edge->neighbour);
    for (auto edge = center_back->previous; edge != nullptr;
         edge = prev(edge)) {
        if (edge->target == pivot) {
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
 * @param[in] center The center node.
 * @return Container of nodes adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<const Node *> adjacent_nodes(const Node &center)
{
    auto full_circle = [](const auto &edges) -> bool {
        return (edges.front()->previous->target) == (edges.back()->target);
    };

    auto edges = opposite_edges(center);
    auto result = std::deque<const Node *>{};

    std::transform(
        std::cbegin(edges),
        std::cend(edges),
        std::back_inserter(result),
        [](const auto &edge) { return edge->target; });

    if (!full_circle(edges)) {  // add the first node explicitly
        result.push_front(edges.front()->previous->target);
    }

    return result;
}

/** Attempts to list all adjacent triangles in edge order.
 * @todo Currently cannot deal with non-manifold vertices.
 * @see lod::graph::opposite_edges().
 * @param[in] center The center node.
 * @return Container of triangles adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<Triangle> adjacent_triangles(const Node &center)
{
    auto edges = opposite_edges(center);
    auto result = std::deque<Triangle>{};

    std::transform(
        std::cbegin(edges),
        std::cend(edges),
        std::back_inserter(result),
        [](const auto &edge) { return edge->triangle_edges(); });
    return result;
}
}  // namespace graph
}  // namespace lod
