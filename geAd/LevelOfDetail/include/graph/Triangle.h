#ifndef TRIANGLE_H_J8AYUG7K
#define TRIANGLE_H_J8AYUG7K
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesh graph component: Triangle.
 */

#include <array>

#include "Edge.h"

namespace lod {
namespace graph {
/// @brief Triangle nodes in edge order.
using Triangle = std::array<const DirectedEdge *, 3>;
}  // namespace graph
}  // namespace lod


#endif /* end of include guard: TRIANGLE_H_J8AYUG7K */
