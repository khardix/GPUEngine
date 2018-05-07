#ifndef EDGE_COLLAPSE_H_BBWHPN49
#define EDGE_COLLAPSE_H_BBWHPN49
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Edde collapse operators.
 */

#include <unordered_set>

#include "../graph/Edge.h"
#include "../graph/Mesh.h"
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
    using result_type = graph::Mesh::EdgeSet;

    /// @brief Apply the operator to a mesh element.
    result_type operator()(graph::Mesh &mesh, const operation_type &operation);

    /// @brief Check for problematic collapse on a mesh boundary
    static bool boundary_collapse(const graph::DirectedEdge &collapsed);
};

extern template class EdgeCollapse<operation::HalfEdgeTag>;
}  // namespace oper
}  // namespace lod

#endif /* end of include guard: EDGE_COLLAPSE_H_BBWHPN49 */
