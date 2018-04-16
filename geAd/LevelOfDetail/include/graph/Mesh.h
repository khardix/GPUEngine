#ifndef MESH_H_VR9OG0AK
#define MESH_H_VR9OG0AK
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Graph representation of triangle mesh.
 */

#include <memory>
#include <unordered_set>

#include <geSG/Mesh.h>

#include "../util/hash_combinator.h"
#include "Edge.h"
#include "Node.h"

namespace lod {
namespace graph {
class Mesh {
public:
    using NodeSet = std::unordered_set<Node>;
    using EdgeSet = std::unordered_set<std::unique_ptr<DirectedEdge>>;

    /// @brief Create graph from prepared sets.
    explicit Mesh(NodeSet nodes, EdgeSet edges);

private:
    std::unordered_set<Node>                          m_nodes = {};
    std::unordered_set<std::unique_ptr<DirectedEdge>> m_edges = {};
};
}  // namespace graph
}  // namespace lod


// Inline and template members
/** Move the prepared sets to the new Mesh.
 * @param[in] nodes Set of nodes for the new graph.
 * @param[in] edges Set of edges for the new graph.
 */
inline lod::graph::Mesh::Mesh(NodeSet nodes, EdgeSet edges)
    : m_nodes(std::move(nodes)), m_edges(std::move(edges))
{
}

#endif /* end of include guard: MESH_H_VR9OG0AK */
