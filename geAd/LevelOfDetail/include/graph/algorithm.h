#ifndef ALGORITHM_H_SMHGX2NT
#define ALGORITHM_H_SMHGX2NT
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Generic graph algorithms.
 *
 * The purpose of this file is to break circular dependency between Node.h and
 * Edge.h, and provide prepared graph traversal and analysis algorithms.
 */

#include <array>
#include <deque>
#include <stdexcept>

#include "Edge.h"
#include "Node.h"
#include "Triangle.h"

namespace lod {
namespace graph {
/// @brief An algorithm cannot process the passed topology.
struct algorithm_failure : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// @brief List all edges opposite to center node.
std::deque<DirectedEdge *> opposite_edges(const Node &center);

/// @brief List all adjacent nodes in edge order.
std::deque<const Node *> adjacent_nodes(const Node &center);

/// @brief List all adjacent triangles in edge order.
std::deque<Triangle> adjacent_triangles(const Node &center);
}  // namespace graph
}  // namespace lod

#endif /* end of include guard: ALGORITHM_H_SMHGX2NT */
