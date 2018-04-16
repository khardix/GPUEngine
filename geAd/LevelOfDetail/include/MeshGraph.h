#ifndef MESHGRAPH_H_PB5REYZQ
#define MESHGRAPH_H_PB5REYZQ

/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Graph representation of triangle mesh.
 */

#include <functional>
#include <utility>
#include <vector>

#ifndef variant_CONFIG_MAX_ALIGN_HACK
#define variant_CONFIG_MAX_ALIGN_HACK 1
#endif  // variant_CONFIG_MAX_ALIGN_HACK
// ^ silence variant warning, should have no effect on c++11 and later
#include <nonstd/variant.hpp>

#include <glm/glm.hpp>

#include "util/hash_combinator.h"

namespace lod {
namespace graph {
struct Node;
struct DirectedEdge;
class UndirectedEdge;

/// @brief Single vertex with adjacency information.
struct Node {
    friend struct std::hash<Node>;

    glm::vec3 position = {0.f, 0.f, 0.f};  ///< Vertex position in model space.
    DirectedEdge *edge = nullptr;          ///< Arbitrary first outgoing edge.

    bool operator==(const Node &other) const noexcept;
    bool operator!=(const Node &other) const noexcept;
};

/// @brief Half-edge with adjacency information.
struct DirectedEdge {
    /// @brief Mesh strucutre error indicators.
    enum class invalid {
        boundary,    ///< Edge is on a boundary.
        nonmanifold  ///< Too many triangles share an edge.
    };

    /// @brief Possibly invalid edge reference.
    using MaybeEdge = nonstd::variant<DirectedEdge *, invalid>;

    Node *        target = nullptr;    ///< Target vertex.
    DirectedEdge *previous = nullptr;  ///< Previous edge in polygon.
    MaybeEdge     neigbour = nullptr;  ///< Opposite direction half-edge.
};

/// @brief Hashable canonical representation of an edge.
class UndirectedEdge {
public:
    friend struct std::hash<UndirectedEdge>;

    /// @brief Reference existing directed edge.
    explicit UndirectedEdge(DirectedEdge &edge) noexcept;

    bool operator==(const UndirectedEdge &other) const noexcept;
    bool operator!=(const UndirectedEdge &other) const noexcept;

protected:
    /// @brief Extract boundary nodes in canonical order.
    std::pair<const Node *, const Node *> nodes() const;

private:
    std::reference_wrapper<DirectedEdge> m_edge;  ///< The wrapped half-edge.
};
}  // namespace graph
}  // namespace lod


/* Inline and template members. {{{ */
/// @brief Nodes are considered equal if they are at the same position.
inline bool lod::graph::Node::operator==(const Node &other) const noexcept
{
    return glm::all(glm::equal(position, other.position));
}
inline bool lod::graph::Node::operator!=(const Node &other) const noexcept
{
    return !(*this == other);
}

/** Wraps a reference to existing edge,
 * and provides comparison and hash semantic for the corresponding full edge.
 * @param[in] edge Reference to existing directed edge.
 */
inline lod::graph::UndirectedEdge::UndirectedEdge(DirectedEdge &edge) noexcept
    : m_edge(std::ref(edge))
{
}

/** Edge's nodes in memory order. */
inline std::pair<const lod::graph::Node *, const lod::graph::Node *>
lod::graph::UndirectedEdge::nodes() const
{
    const auto *n1 = m_edge.get().target;
    const auto *n2 = m_edge.get().previous->target;
    return std::make_pair(std::min(n1, n2), std::max(n1, n2));
}

/** All edges between the same two nodes are equal. */
inline bool lod::graph::UndirectedEdge::operator==(
    const UndirectedEdge &other) const noexcept
{
    return nodes() == other.nodes();
}
inline bool lod::graph::UndirectedEdge::operator!=(
    const UndirectedEdge &other) const noexcept
{
    return !(*this == other);
}

namespace std {
template <>
struct hash<lod::graph::Node> {
    using argument_type = lod::graph::Node;
    using return_type = std::size_t;

    /// @brief Node is hashed by its position.
    return_type operator()(const argument_type &node) const noexcept
    {
        return lod::util::hash_combinator(
            0, node.position.x, node.position.y, node.position.z);
    }
};

template <>
struct hash<lod::graph::UndirectedEdge> {
    using argument_type = lod::graph::UndirectedEdge;
    using return_type = std::size_t;

    /** @brief Calculate edge hash.
     * The hash should be comutative: `hash(AB) == hash(BA)`.
     */
    return_type operator()(const argument_type &edge) const noexcept
    {
        const auto nodes = edge.nodes();
        return lod::util::hash_combinator(0, nodes.first, nodes.second);
    }
};
}  // namespace std
/* }}} Inline and template members. */

#endif /* end of include guard: MESHGRAPH_H_PB5REYZQ */
