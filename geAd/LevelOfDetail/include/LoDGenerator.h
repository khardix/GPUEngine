#ifndef LODGENERATOR_H_79EHVANO
#define LODGENERATOR_H_79EHVANO
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Main header file of the addon, exposing the "public" API.
 */

#include "algorithm/lazy_selection.h"
#include "graph/Mesh.h"
#include "metric/qem.h"
#include "operator/edge_collapse.h"
#include "protocol.h"

namespace lod {
// Graph API
using graph::Mesh;

// Protocol API
using operation::FullEdgeTag;
using operation::HalfEdgeTag;

// Metric API
using metric::QEM;

// Operator API
using oper::EdgeCollapse;

// Algorithm API
using algorithm::lazy_selection;
using algorithm::LazySelection;

/// @brief Prepared simplification function.
/// @see LazySelection for full range of possible call signatures.
template <
    typename Tag = FullEdgeTag,
    template <typename> class Metric = QEM,
    template <typename> class Operator = EdgeCollapse,
    typename... Args>
inline decltype(auto) simplify(Args &&... args)
{
    return lazy_selection<Tag, Metric, Operator>(std::forward<Args>(args)...);
}
}  // namespace lod

#endif /* end of include guard: LODGENERATOR_H_79EHVANO */
