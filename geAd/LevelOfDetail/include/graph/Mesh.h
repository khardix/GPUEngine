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

#include "../protocol.h"
#include "../util/hash_combinator.h"
#include "Edge.h"
#include "Node.h"

namespace lod {
namespace graph {
class Mesh {
public:
    using NodeSet = std::unordered_set<Node::pointer_type>;
    using EdgeSet = std::unordered_set<DirectedEdge::pointer_type>;
    using EdgeCache = std::unordered_set<UndirectedEdge>;

    /// @brief Create graph from prepared sets.
    explicit Mesh(NodeSet &&nodes, EdgeSet &&edges);
    /// @brief Create graph from existing mesh.
    explicit Mesh(const ge::sg::Mesh &original);

    Mesh(const Mesh &) = delete;
    Mesh(Mesh &&) = default;
    Mesh &operator=(const Mesh &) = delete;
    Mesh &operator=(Mesh &&) = default;

    /// @brief Create an equivalent ge::sg::Mesh.
    explicit operator ge::sg::Mesh() const;

    /// @brief Access to stored nodes.
    NodeSet &      nodes() noexcept;
    const NodeSet &nodes() const noexcept;
    /// @brief Access to stored edges.
    EdgeSet &      edges() noexcept;
    const EdgeSet &edges() const noexcept;

    /// @brief Access the containter of selected type.
    template <typename Element>
    const std::unordered_set<Element> &container() const noexcept;

protected:
    /// @brief Insert single triangle into a graph.
    void insert(const ge::sg::Triangle &, EdgeCache &cache);

private:
    NodeSet m_nodes = {};
    EdgeSet m_edges = {};
};


// Inline and template members
/** Move the prepared sets to the new Mesh.
 * @param[in] nodes Set of nodes for the new graph.
 * @param[in] edges Set of edges for the new graph.
 */
inline Mesh::Mesh(NodeSet &&nodes, EdgeSet &&edges)
    : m_nodes(std::move(nodes)), m_edges(std::move(edges))
{
}

inline Mesh::NodeSet &Mesh::nodes() noexcept
{
    return m_nodes;
}
inline const Mesh::NodeSet &Mesh::nodes() const noexcept
{
    return m_nodes;
}
inline Mesh::EdgeSet &Mesh::edges() noexcept
{
    return m_edges;
}
inline const Mesh::EdgeSet &Mesh::edges() const noexcept
{
    return m_edges;
}

template <>
inline const Mesh::NodeSet &Mesh::container<Node::pointer_type>() const noexcept
{
    return m_nodes;
}
template <>
inline const Mesh::EdgeSet &Mesh::container<DirectedEdge::pointer_type>() const
    noexcept
{
    return m_edges;
}
}  // namespace graph
}  // namespace lod

#endif /* end of include guard: MESH_H_VR9OG0AK */
