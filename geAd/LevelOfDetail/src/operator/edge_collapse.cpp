/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Edde collapse operators (implementation).
 */

#include <algorithm>
#include <exception>
#include <iterator>
#include <unordered_map>
#include <utility>

#include <glm/glm.hpp>

#include <graph/Node.h>
#include <graph/algorithm.h>
#include <operator/edge_collapse.h>
#include <util/set_operations.h>

namespace lod {
namespace oper {
using operation::FullEdgeTag;
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
graph::DirectedEdge::pointer_type connect_neighbours(
    const graph::DirectedEdge::pointer_type &lhs,
    const graph::DirectedEdge::pointer_type &rhs)
{
    using lod::graph::DirectedEdge;

    auto lneigh = nonstd::get<DirectedEdge::weak_type>(lhs->neighbour()).lock();
    auto rneigh = nonstd::get<DirectedEdge::weak_type>(rhs->neighbour()).lock();

    if (lneigh != nullptr) {
        lneigh->neighbour() = rneigh;
    }
    if (rneigh != nullptr) {
        rneigh->neighbour() = lneigh;
    }

    return lneigh == nullptr ? rneigh : lneigh;
}

/** Marks edges for deletion:
 * 1. Resets reference on nodes that point to them.
 * 2. Insert edges to trash set.
 * @param[in,out] trash The set to insert deleted edges to.
 * @param[in] triangle The triangle to delete.
 */
void mark_triangle_deleted(
    graph::Mesh::EdgeSet &trash, const graph::DetachedTriangle &triangle)
{
    for (const auto &edge : triangle) {
        auto &source_emanating_edge
            = edge->previous().lock()->target().lock()->edge();
        if (source_emanating_edge.lock() == edge) {
            source_emanating_edge.reset();
        }

        trash.insert(edge);
    }
}

/** Checks for possible folds in mesh by measuring the angle of normal vectors
 * of original and candidate triangle.
 * @param[in] opposite The edge common to both triangles, opposite to the nodes.
 * @param[in] current The last node of current triangle.
 * @param[in] candidate The last node of candidate triangle.
 * @returns True if there is a high probability of fold (∠ ≥ 90°),
 * false otherwise.
 */
bool common::EdgeCollapse::would_fold(
    const graph::DirectedEdge &opposite,
    const graph::Node &        current,
    const graph::Node &        candidate)
{
    const auto &origin_pos
        = opposite.previous().lock()->target().lock()->position();
    const auto &target_pos = opposite.target().lock()->position();
    const auto &current_pos = current.position();
    const auto &candidate_pos = candidate.position();

    const auto current_normal = glm::normalize(
        glm::cross(origin_pos - current_pos, target_pos - current_pos));
    const auto candidate_normal = glm::normalize(
        glm::cross(origin_pos - candidate_pos, target_pos - candidate_pos));

    const auto angle
        = glm::degrees(glm::acos(glm::dot(current_normal, candidate_normal)));

    return angle >= 90.0;
}

/** Checks for creation of non-manifold edges by counting common neighbour
 * nodes. If the collapsed nodes have 3 or more common adjacent nodes, there
 * will be (at least one) non-manifold edge after the collapse.
 * @param[in] collapsed The collapsed edge.
 * @return True if the collapse would cause non-manifold edge, false otherwise.
 */
bool common::EdgeCollapse::nonmanifold_collapse(
    const graph::DirectedEdge &collapsed)
{
    using namespace lod::graph;
    using trans = std::pair<Node::pointer_type, Mesh::NodeSet &>;

    auto origin_nodes = Mesh::NodeSet{}, target_nodes = Mesh::NodeSet{};
    for (auto &&t :
         {trans{collapsed.previous().lock()->target().lock(), origin_nodes},
          trans{collapsed.target().lock(), target_nodes}}) {
        auto container = adjacent_nodes(*t.first);
        std::transform(
            std::begin(container),
            std::end(container),
            std::inserter(t.second, t.second.end()),
            [](auto &&node_ptr) -> Node::pointer_type {
                return node_ptr.lock();
            });
    }

    const auto common_nodes = util::intersection(origin_nodes, target_nodes);
    return common_nodes.size() >= 3;
}

/** Half-edge collapse should not move original mesh border.
 * It can result in degenerated triangles,
 * inter-mesh discontinuity and similar problems.
 * @param[in] collapsed The edge to be collapsed.
 * @return True if the collapse would move a boundary edge.
 */
bool EdgeCollapse<HalfEdgeTag>::boundary_collapse(
    const graph::DirectedEdge &collapsed)
{
    using namespace lod::graph;

    if (collapsed.boundary()) {
        return true;
    }

    auto other_edges
        = emanating_edges(*collapsed.previous().lock()->target().lock());
    return std::any_of(
        std::cbegin(other_edges), std::cend(other_edges), [](const auto &edge) {
            return edge->boundary();
        });
}

/** Half-edge collapse should not move original mesh border.
 * It can result in degenerated triangles,
 * inter-mesh discontinuity and similar problems.
 * @param[in] collapsed The edge to be collapsed.
 * @return True if the collapse would move a boundary edge.
 */
bool EdgeCollapse<FullEdgeTag>::boundary_collapse(
    const graph::DirectedEdge &collapsed)
{
    using namespace lod::graph;

    if (collapsed.boundary()) {
        return true;
    }

    auto tgt_edges = emanating_edges(*collapsed.target().lock());
    auto org_edges
        = emanating_edges(*collapsed.previous().lock()->target().lock());
    auto is_boundary = [](const auto &edge) { return edge->boundary(); };

    return std::any_of(
               std::cbegin(tgt_edges), std::cend(tgt_edges), is_boundary)
        || std::any_of(
               std::cbegin(org_edges), std::cend(org_edges), is_boundary);
}

/** Applies the operator to the mesh.
 * @param[in,out] state The current state of the decimation.
 * @param operation The operation to perform.
 * @returns Set of elements influenced by the operation.
 * @throws graph::algorithm_failure On encounter with non-manifold edge.
 * @todo If the operation cannot be safely applied, it is silently skipped.
 */
void EdgeCollapse<HalfEdgeTag>::operator()(
    SimplificationState<Tag::element_type> &state,
    const operation_type &                  operation) const try {
    using namespace lod::graph;

    auto &modified = state.dirty();
    auto  edges_to_delete = Mesh::EdgeSet{};

    const auto &collapsed_edge = operation.element().get();
    const auto  target_node = collapsed_edge->target().lock();
    const auto origin_node = collapsed_edge->previous().lock()->target().lock();

    auto       edge_ring = opposite_edges(*origin_node);
    const auto edge_ring_complete
        = (edge_ring.front()->previous().lock()->target().lock()
           == edge_ring.back()->target().lock());

    /// This triangle will be replaced by previous one in the ring.
    auto replaced_by_prev = [&target_node](const auto &ring_edge) -> bool {
        return ring_edge->target().lock() == target_node;
    };
    /// This triangle will be replaced by next one in the ring.
    auto replaced_by_next = [&target_node](const auto &ring_edge) -> bool {
        return ring_edge->previous().lock()->target().lock() == target_node;
    };

    // Make preliminary checks for operation validity
    if (nonmanifold_collapse(*collapsed_edge)
        || boundary_collapse(*collapsed_edge)
        || would_fold(
               std::cbegin(edge_ring),
               std::cend(edge_ring),
               *origin_node,
               *target_node)) {
        return;
    }

    // Apply the edge adjustments
    for (auto &&opposite : edge_ring) {
        if (replaced_by_prev(opposite)) {
            mark_triangle_deleted(edges_to_delete, opposite->triangle_edges());
            // replace deleted edge with valid neighbour
            opposite
                = connect_neighbours(opposite->previous().lock(), opposite);
            continue;
        }

        if (replaced_by_next(opposite)) {
            mark_triangle_deleted(edges_to_delete, opposite->triangle_edges());
            // replace deleted edge with valid neighbour
            opposite = connect_neighbours(opposite->next(), opposite);
            continue;
        }

        // connect outgoing edge to new target
        opposite->next()->target() = target_node;

        // mark all edges as modified
        auto triangle = opposite->triangle_edges();
        modified.insert(std::cbegin(triangle), std::cend(triangle));
    }

    // fix nodes with deleted references
    for (const auto &opposite : edge_ring) {
        if (opposite == nullptr) {
            throw std::runtime_error("Non-manifold collapse!");
        }

        auto node = opposite->target().lock();
        if (node->edge().expired()) {
            node->edge() = opposite->next();
        }
    }

    if (!edge_ring_complete) {
        auto first_node = edge_ring.front()->previous().lock()->target().lock();
        if (first_node->edge().expired()) {
            first_node->edge() = edge_ring.front();
        }
    }

    // remove collapsed node and deleted edges
    state.mark_deleted(origin_node);
    for (auto &&deleted : edges_to_delete) {
        state.mark_deleted(deleted);
    }
}
catch (const nonstd::bad_variant_access &) {
    auto message = graph::algorithm_failure("Non-manifold half-edge collapse!");
    std::throw_with_nested(message);
}

/** Applies the operator to the mesh.
 * @param[in,out] state The current state of the simplification.
 * @param operation The operation to perform.
 * @returns Set of elements influenced by the operation.
 * @throws graph::algorithm_failure On encounter with non-manifold edge.
 * @todo If the operation cannot be safely applied, it is silently skipped.
 */
void EdgeCollapse<FullEdgeTag>::operator()(
    SimplificationState<Tag::element_type> &state,
    const operation_type &                  operation) const try {
    using namespace lod::graph;

    auto collapsed = operation.element().get();
    auto opposite
        = nonstd::get<DirectedEdge::weak_type>(collapsed->neighbour()).lock();

    auto       candidate = Node::make(operation.position_hint());
    const auto target_node = collapsed->target().lock();
    const auto origin_node = collapsed->previous().lock()->target().lock();

    auto &modified = state.dirty();
    auto  edges_to_delete = Mesh::EdgeSet{};

    /// Make an edge ring without edges touching the other node.
    auto partial_ring = [](const auto &center_edge) {
        const auto target = center_edge->target().lock();
        const auto origin = center_edge->previous().lock()->target().lock();

        auto ring = opposite_edges(*origin);
        auto new_end = std::remove_if(
            std::begin(ring), std::end(ring), [&](const auto &edge) {
                return edge->target().lock() == target
                    || edge->previous().lock()->target().lock() == target;
            });
        ring.erase(new_end, std::end(ring));

        return ring;
    };

    // preliminary checks of operation validity
    if (nonmanifold_collapse(*collapsed) || boundary_collapse(*collapsed)) {
        return;
    }
    for (const auto &edge : {collapsed, opposite}) {
        if (!edge) {
            continue;
        }

        auto ring = partial_ring(edge);
        auto fold = would_fold(
            std::cbegin(ring),
            std::cend(ring),
            *edge->target().lock(),
            *candidate);

        if (fold) {
            return;
        }
    }

    // Insert and connect the candidate
    const auto &new_node
        = *state.mesh().nodes().insert(std::move(candidate)).first;
    for (const auto &edge : {collapsed, opposite}) {
        if (!edge) {
            continue;
        }

        auto triangle = edge->triangle_edges();
        connect_neighbours(triangle[1], triangle[2]);

        if (new_node->edge().expired()) {
            auto outgoing
                = nonstd::get<DirectedEdge::weak_type>(triangle[2]->neighbour())
                      .lock();

            if (outgoing) {
                new_node->edge() = outgoing;
            }
        }

        mark_triangle_deleted(edges_to_delete, triangle);
    }

    // Adjust the surroundings
    for (const auto &edge : opposite_edges(*new_node)) {
        edge->next()->target() = new_node;
        if (edge->target().lock()->edge().expired()) {
            edge->target().lock()->edge() = edge->next();
        }

        auto triangle = edge->triangle_edges();
        modified.insert(std::cbegin(triangle), std::cend(triangle));
    }

    // Drop nodes and edges
    if (new_node != target_node) {
        state.mark_deleted(target_node);
    }
    if (new_node != origin_node) {
        state.mark_deleted(origin_node);
    }
    for (auto &&edge : edges_to_delete) {
        state.mark_deleted(edge);
    }
}
catch (const nonstd::bad_variant_access &) {
    auto message = graph::algorithm_failure("Non-manifold full-edge collapse!");
    std::throw_with_nested(message);
}

template class EdgeCollapse<HalfEdgeTag>;
template class EdgeCollapse<FullEdgeTag>;
}  // namespace oper
}  // namespace lod
