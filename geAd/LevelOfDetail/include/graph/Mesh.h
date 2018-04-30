#ifndef MESH_H_VR9OG0AK
#define MESH_H_VR9OG0AK
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Graph representation of triangle mesh.
 */

#include <array>
#include <memory>
#include <unordered_set>

#include <geSG/Mesh.h>
#include <geSG/MeshTriangleIterators.h>

#include "../util/hash_combinator.h"
#include "Edge.h"
#include "Node.h"

namespace lod {
namespace graph {
class Mesh {
public:
    using NodeSet = std::unordered_set<Node>;
    using EdgeSet = std::unordered_set<std::unique_ptr<DirectedEdge>>;
    using EdgeCache = std::unordered_set<UndirectedEdge>;

    /// @brief Create graph from prepared sets.
    explicit Mesh(NodeSet nodes, EdgeSet edges);
    /// @brief Create graph from existing mesh.
    explicit Mesh(/*const*/ ge::sg::Mesh &original);

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    /// @brief Create an equivalent ge::sg::Mesh.
    explicit operator ge::sg::Mesh() const;

    /// @brief Access to stored nodes.
    const NodeSet &nodes() const noexcept;
    /// @brief Access to stored edges.
    const EdgeSet &edges() const noexcept;

protected:
    /// @brief Insert single triangle into a graph.
    void insert(const ge::sg::Triangle &, EdgeCache &cache);

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

inline const lod::graph::Mesh::NodeSet &lod::graph::Mesh::nodes() const noexcept
{
    return m_nodes;
}
inline const lod::graph::Mesh::EdgeSet &lod::graph::Mesh::edges() const noexcept
{
    return m_edges;
}

#endif /* end of include guard: MESH_H_VR9OG0AK */
