/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Edde collapse operators (implementation).
 */

#include <algorithm>
#include <exception>
#include <iterator>
#include <unordered_map>
#include <utility>

#include <graph/Node.h>
#include <graph/algorithm.h>
#include <operator/edge_collapse.h>
#include <util/set_operations.h>

namespace lod {
namespace oper {
using operation::HalfEdgeTag;

/** Applies the operator to the mesh.
 * @param mesh The mesh to be modified.
 * @param operation The operation to perform.
 * @returns Set of elements influenced by the operation.
 * @throws graph::algorithm_failure On encounter with non-manifold edge.
 * @todo If the operation cannot be safely applied, it is silently skipped.
 */
auto EdgeCollapse<HalfEdgeTag>::operator()(
    graph::Mesh &mesh, const operation_type &operation) -> result_type try {
    using namespace lod::graph;

    auto &mesh_edges = mesh.edges();

    auto modified = result_type{};
    auto to_delete = Mesh::EdgeSet{};

    const auto edge = operation.element().get();
    const auto target = edge->target;
    const auto origin = edge->previous->target;

    auto edge_ring = opposite_edges(*origin);

    auto connect_neighbours = [](const auto &lhs, const auto &rhs) {
        auto lneigh = nonstd::get<DirectedEdge *>(lhs->neighbour);
        auto rneigh = nonstd::get<DirectedEdge *>(rhs->neighbour);

        if (lneigh != nullptr) {
            lneigh->neighbour = rneigh;
        }
        if (rneigh != nullptr) {
            rneigh->neighbour = lneigh;
        }

        return lneigh == nullptr ? rneigh : lneigh;
    };

    auto mark_triangle_deleted = [&mesh_edges, &to_delete](auto &&start_edge) {
        for (auto &&edge : start_edge->triangle_edges()) {
            if (edge->previous->target->edge == edge) {
                edge->previous->target->edge = nullptr;
            }

            auto edge_iter = mesh_edges.find(util::elevate(edge));
            if (edge_iter == mesh_edges.end()) {
                continue;
            }
            to_delete.insert(*edge_iter);
        }
    };

    // Apply the edge adjustments
    for (auto &&opposite : edge_ring) {
        if (opposite->target == target) {  // replaced by previous triangle
            mark_triangle_deleted(opposite);
            opposite = connect_neighbours(opposite->previous, opposite);
            continue;
        }

        if (opposite->previous->target == target) {
            // replaced by next triangle
            mark_triangle_deleted(opposite);
            opposite = connect_neighbours(opposite->next(), opposite);
            continue;
        }

        // connect outgoing edge to new target
        auto next_edge = opposite->next();
        next_edge->target = target;

        // mark both edges as modified
        modified.insert({next_edge, opposite->previous});
    }

    // fix nodes with deleted references
    for (const auto &opposite : edge_ring) {
        if (opposite == nullptr) {
            throw std::runtime_error("Cannot collapse last two triangles!");
        }

        if (opposite->target->edge == nullptr) {
            opposite->target->edge = opposite->next();
        }
    }

    // remove collapsed node and deleted edges
    mesh.nodes().erase(*origin);
    for (auto &&deleted : to_delete) {
        mesh.edges().erase(deleted);
    }

    return modified;
}
catch (const nonstd::bad_variant_access &) {
    auto message = graph::algorithm_failure("Non-manifold edge collapse!");
    std::throw_with_nested(message);
}

template class EdgeCollapse<HalfEdgeTag>;
}  // namespace oper
}  // namespace lod
