/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Generic graph algorithms (implementation).
 */

#include <cassert>
#include <exception>

#include <graph/algorithm.h>

namespace lod {
namespace graph {
/** Attempts to list all adjacent nodes in "edge order"
 * (source Node before Target node for each edge opposite to center).
 * @todo Currently cannot deal with non-manifold vertices.
 * @see Campagna et al.: Directed Edges - A Scalable Representation for Triangle
 * Meshes, 1998.
 * @param[in] center The center node.
 * @return Vector of nodes adjacent to the center node.
 * @throws algorithm_failure On encounter with non-manifold edge.
 */
std::deque<const Node *> adjacent_nodes(const Node &center) try {
    if (center.edge == nullptr) {
        throw std::runtime_error("Sole node!");
    }
    auto result = std::deque<const Node *>{center.edge->target};

    const auto next = [](const DirectedEdge *const edge) -> DirectedEdge * {
        auto neigh = nonstd::get<DirectedEdge *>(edge->next()->neighbour);
        return neigh == nullptr ? nullptr : neigh->next();
    };
    const auto prev = [](const DirectedEdge *const edge) -> DirectedEdge * {
        auto neigh = nonstd::get<DirectedEdge *>(edge->previous->neighbour);
        return neigh == nullptr ? nullptr : neigh->previous;
    };

    // walk until a mesh boundary or full circle
    auto pivot = result.front();
    for (auto edge = center.edge->next(); edge != nullptr; edge = next(edge)) {
        if (edge->target == pivot) {  // full circle
            return result;
        }
        result.push_back(edge->target);
    }

    // stopped at boundary; check if we need to walk backwards as well
    if (center.edge->boundary()) {
        return result;
    }

    pivot = result.back();
    auto center_back = nonstd::get<DirectedEdge *>(center.edge->neighbour);
    for (auto edge = center_back->previous; edge != nullptr;
         edge = prev(edge)) {
        if (edge->target == pivot) {
            return result;
        }
        result.push_front(edge->target);
    }

    return result;
}
catch (const nonstd::bad_variant_access &) {
    auto message = algorithm_failure("Non-manifold edge in adjecent_nodes");
    std::throw_with_nested(message);
}
}  // namespace graph
}  // namespace lod
