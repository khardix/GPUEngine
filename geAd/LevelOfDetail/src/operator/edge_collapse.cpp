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

/** Connects neighbours of two edges to each other.
 * Combined with correct edge adjustments,
 * this has the effect of removing a triangle from the mesh.
 * @param[in] lhs One of the edges to detach.
 * @param[in] rhs The other edge to detach.
 * @returns Pointer to one of the neighbours.
 * If one of the input edges was on boundary,
 * returns the neighbour of the other one (== valid neighbour).
 * If both edges were boundary, returns nullptr.
 */
graph::DirectedEdge *connect_neighbours(
    graph::DirectedEdge *const lhs, graph::DirectedEdge *const rhs)
{
    auto lneigh = nonstd::get<graph::DirectedEdge *>(lhs->neighbour);
    auto rneigh = nonstd::get<graph::DirectedEdge *>(rhs->neighbour);

    if (lneigh != nullptr) {
        lneigh->neighbour = rneigh;
    }
    if (rneigh != nullptr) {
        rneigh->neighbour = lneigh;
    }

    return lneigh == nullptr ? rneigh : lneigh;
}

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
    const auto target_node = edge->target;
    const auto origin_node = edge->previous->target;

    auto       edge_ring = opposite_edges(*origin_node);
    const auto edge_ring_complete
        = edge_ring.front()->previous->target == edge_ring.back()->target;

    /* Marks all edges from a triangle for deletion,
     * and remove them from nodes that refer to them.
     */
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
        // replaced by previous triangle
        if (opposite->target == target_node) {
            mark_triangle_deleted(opposite);
            // replace deleted edge with valid neighbour
            opposite = connect_neighbours(opposite->previous, opposite);
            continue;
        }

        // replaced by next triangle
        if (opposite->previous->target == target_node) {
            mark_triangle_deleted(opposite);
            // replace deleted edge with valid neighbour
            opposite = connect_neighbours(opposite->next(), opposite);
            continue;
        }

        // connect outgoing edge to new target
        auto next_edge = opposite->next();
        next_edge->target = target_node;

        // mark all edges as modified
        modified.insert({next_edge, opposite, opposite->previous});
    }

    // fix nodes with deleted references
    for (const auto &opposite : edge_ring) {
        if (opposite == nullptr) {
            throw std::runtime_error("Non-manifold collapse!");
        }

        auto node = opposite->target;
        if (node->edge == nullptr) {
            node->edge = opposite->next();
        }
    }

    if (!edge_ring_complete) {
        auto &first_node = edge_ring.front()->previous->target;
        if (first_node->edge == nullptr) {
            first_node->edge = edge_ring.front();
        }
    }

    // remove collapsed node and deleted edges
    mesh.nodes().erase(*origin_node);
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
