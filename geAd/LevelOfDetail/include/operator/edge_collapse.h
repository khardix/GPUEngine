#ifndef EDGE_COLLAPSE_H_BBWHPN49
#define EDGE_COLLAPSE_H_BBWHPN49
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Edde collapse operators.
 */

#include <unordered_set>

#include "../graph/Edge.h"
#include "../protocol.h"

namespace lod {
namespace oper {  // operator is reserved keyword
namespace common {
/// @brief Functionality not dependent on operation type.
struct EdgeCollapse {
    /// @brief Check for possible mesh fold.
    static bool would_fold(
        const graph::DirectedEdge &opposite,
        const graph::Node &        current,
        const graph::Node &        candidate);
    template <typename InputIt>
    static bool would_fold(
        InputIt            op_begin,
        InputIt            op_end,
        const graph::Node &current,
        const graph::Node &candidate);
    /// @brief Check for possible creation of non-manifold edges.
    static bool nonmanifold_collapse(const graph::DirectedEdge &collapsed);
};
}  // namespace common

template <typename Tag>
class EdgeCollapse;

/// @brief Half-Edge collapse operator.
template <>
class EdgeCollapse<operation::HalfEdgeTag> : common::EdgeCollapse {
public:
    using Tag = operation::HalfEdgeTag;
    using operation_type = operation::Simple<Tag::element_type>;

    /// @brief Apply the operator to a mesh element.
    void operator()(
        SimplificationState<Tag::element_type> &state,
        const operation_type &                  operation) const;

    /// @brief Check for problematic collapse on a mesh boundary
    static bool boundary_collapse(const graph::DirectedEdge &collapsed);
};

/// @brief Full-Edge collapse operator.
template <>
class EdgeCollapse<operation::FullEdgeTag> : common::EdgeCollapse {
public:
    using Tag = operation::FullEdgeTag;
    using operation_type = operation::VertexPlacement<Tag::element_type>;

    /// @brief Apply the operator to a mesh element.
    void operator()(
        lod::SimplificationState<Tag::element_type> &state,
        const operation_type &                       operation) const;

    /// @brief Check for problematic collapse on a mesh boundary
    static bool boundary_collapse(const graph::DirectedEdge &collapsed);
};

extern template class EdgeCollapse<operation::HalfEdgeTag>;
extern template class EdgeCollapse<operation::FullEdgeTag>;


// Inline and template members
/** @overload
 * Container variant of would_fold.
 * @param[in] op_begin Start of the opposite edges (DirectedEdge::pointer_type).
 * @param[in] op_end End of the opposite edges (DirectedEdge::pointer_type).
 * @param[in] current Current final node in triangle.
 * @param[in] candidate Candidate final node in triangle.
 * @return True if any edge would fold, false otherwise.
 */
template <typename InputIt>
inline bool common::EdgeCollapse::would_fold(
    InputIt            op_begin,
    InputIt            op_end,
    const graph::Node &current,
    const graph::Node &candidate)
{
    return std::any_of(op_begin, op_end, [&](const auto &edge) {
        return would_fold(*edge, current, candidate);
    });
}
}  // namespace oper
}  // namespace lod

#endif /* end of include guard: EDGE_COLLAPSE_H_BBWHPN49 */
