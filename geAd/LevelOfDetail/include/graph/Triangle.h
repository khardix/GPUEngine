#ifndef TRIANGLE_H_J8AYUG7K
#define TRIANGLE_H_J8AYUG7K
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesh graph component: Triangle.
 */

#include <array>
#include <memory>

#include "Edge.h"
#include "Node.h"

namespace lod {
namespace graph {
/// @brief Triangle nodes in edge order.
using Triangle = std::array<const DirectedEdge *, 3>;
/// @brief Triangle not connected to any mesh.
using DetachedTriangle = std::array<std::unique_ptr<DirectedEdge>, 3>;

/// @brief Constructs detached Triangle from three Nodes.
DetachedTriangle make_triangle(const std::array<const Node *, 3> &nodes);
}  // namespace graph
}  // namespace lod


#endif /* end of include guard: TRIANGLE_H_J8AYUG7K */
