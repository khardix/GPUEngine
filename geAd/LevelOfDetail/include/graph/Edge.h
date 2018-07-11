#ifndef EDGE_H_HRCWE1XD
#define EDGE_H_HRCWE1XD
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesg graph component: Edge
 */

#include <array>
#include <functional>
#include <memory>
#include <utility>

// silence variant warning that should have no effect on C++11 and later
#ifndef variant_CONFIG_MAX_ALIGN_HACK
#define variant_CONFIG_MAX_ALIGN_HACK 1
#endif  // variant_CONFIG_MAX_ALIGN_HACK
#include <nonstd/variant.hpp>

#include "../util/hash_combinator.h"
#include "Node.h"

namespace lod {
namespace graph {
/// @brief Half-edge with adjacency information.
class DirectedEdge : public std::enable_shared_from_this<DirectedEdge> {
public:
    /// @brief Mesh strucutre error indicators.
    enum class invalid {
        nonmanifold  ///< Too many triangles share an edge.
    };

    /// @brief Owning reference to another edge.
    using pointer_type = std::shared_ptr<DirectedEdge>;
    using const_pointer_type = std::shared_ptr<const DirectedEdge>;
    /// @brief Non-owning pointer to another edge.
    using weak_type = std::weak_ptr<DirectedEdge>;
    using const_weak_type = std::weak_ptr<const DirectedEdge>;
    /// @brief Possibly invalid edge reference.
    using MaybeEdge = nonstd::variant<weak_type, invalid>;

    /// @brief Enforce shared pointer creation.
    template <typename... Args>
    static pointer_type make(Args &&... args);
    /// @brief "Cast" the edge to shared pointer.
    pointer_type       as_shared() { return shared_from_this(); }
    const_pointer_type as_shared() const { return shared_from_this(); }
    /// @brief "Cast" the edge to weak pointer.
    weak_type       as_weak() { return shared_from_this(); }
    const_weak_type as_weak() const { return shared_from_this(); }

    /// @brief Access stored target.
    Node::const_weak_type &      target() { return m_target; }
    Node::const_weak_type const &target() const { return m_target; }
    /// @brief Access stored previous edge.
    weak_type &      previous() { return m_previous; }
    const weak_type &previous() const { return m_previous; }
    /// @brief Access stored neighbour.
    MaybeEdge &      neighbour() { return m_neighbour; }
    const MaybeEdge &neighbour() const { return m_neighbour; }

    /// @brief Indicates boundary edge.
    bool boundary() const noexcept;
    /// @brief Indicates manifold edge.
    bool manifold() const noexcept;
    /// @brief Calculate next edge in a triangle.
    pointer_type next() const noexcept;
    /// @brief Extract all edges from own triangle.
    std::array<pointer_type, 3>       triangle_edges();
    std::array<const_pointer_type, 3> triangle_edges() const;

protected:
    explicit DirectedEdge(
        Node::const_weak_type target = {},
        weak_type             previous = {},
        MaybeEdge             neighbour = weak_type{}) noexcept;

private:
    Node::const_weak_type m_target = {};    ///< Target vertex.
    weak_type             m_previous = {};  ///< Previous edge in polygon.
    MaybeEdge m_neighbour = weak_type{};    ///< Opposite direction half-edge.
};

/// @brief Hashable canonical representation of an edge.
class UndirectedEdge {
public:
    friend struct std::hash<UndirectedEdge>;

    /// @brief Reference existing directed edge.
    explicit UndirectedEdge(DirectedEdge::pointer_type edge) noexcept;

    bool operator==(const UndirectedEdge &other) const noexcept;
    bool operator!=(const UndirectedEdge &other) const noexcept;

    /// @brief Access referred half-edge
    const DirectedEdge::pointer_type &referred() const;

    /// @brief Extract boundary nodes in canonical order.
    std::pair<Node::const_weak_type, Node::const_weak_type> nodes() const;

private:
    DirectedEdge::pointer_type m_edge;  ///< The wrapped half-edge.
};
}  // namespace graph
}  // namespace lod


// Inline and template members
namespace std {
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
        return lod::util::hash_combinator(
            0, nodes.first.lock(), nodes.second.lock());
    }
};
}  // namespace std

namespace lod {
namespace graph {
inline DirectedEdge::DirectedEdge(
    Node::const_weak_type target,
    weak_type             previous,
    MaybeEdge             neighbour) noexcept
    : m_target(std::move_if_noexcept(target)),
      m_previous(std::move_if_noexcept(previous)),
      m_neighbour(std::move_if_noexcept(neighbour))
{
}

template <typename... Args>
inline auto DirectedEdge::make(Args &&... args) -> pointer_type
{
    return pointer_type{new DirectedEdge(std::forward<Args>(args)...)};
}

/// @return True if the edge is on a mesh boundary, false otherwise.
inline bool DirectedEdge::boundary() const noexcept
{
    return nonstd::holds_alternative<weak_type>(m_neighbour)
        && nonstd::get<weak_type>(m_neighbour).expired();
}

/// @return True if the edge is manifold, false otherwise.
inline bool DirectedEdge::manifold() const noexcept
{
    return !nonstd::holds_alternative<invalid>(m_neighbour)
        || nonstd::get<invalid>(m_neighbour) != invalid::nonmanifold;
}

/** Find an edge where previous == this.
 * @return Pointer to the next edge, or nullptr if the cycle is broken.
 */
inline auto DirectedEdge::next() const noexcept -> pointer_type
{
    if (auto previous = m_previous.lock()) {
        return previous->previous().lock();
    }
    else {
        return nullptr;
    }
}

/** Extract edges in correct order.
 * @return All edges in own triangle in correct order.
 * @throws std::runtime_error On unconnected triangle.
 */
inline auto DirectedEdge::triangle_edges() const
    -> std::array<const_pointer_type, 3>
{
    auto incoming = m_previous.lock();
    auto outgoing = next();
    if (incoming && outgoing) {
        return {shared_from_this(), outgoing, incoming};
    }
    throw std::runtime_error("Unconnected edges!");
}
/// @overload
inline auto DirectedEdge::triangle_edges() -> std::array<pointer_type, 3>
{
    auto incoming = m_previous.lock();
    auto outgoing = next();
    if (incoming && outgoing) {
        return {shared_from_this(), outgoing, incoming};
    }
    throw std::runtime_error("Unconnected edges!");
}

/** Wraps a reference to existing edge,
 * and provides comparison and hash semantic for the corresponding full edge.
 * @param[in] edge Reference to existing directed edge.
 */
inline UndirectedEdge::UndirectedEdge(DirectedEdge::pointer_type edge) noexcept
    : m_edge(std::move(edge))
{
}

/**
 * @returns Edge's nodes in memory order.
 * @throws std::runtime_error Edge is not part of a triangle.
 */
inline std::pair<Node::const_weak_type, Node::const_weak_type>
UndirectedEdge::nodes() const
{
    static const auto cmp = std::owner_less<Node::const_weak_type>{};

    if (auto prev = m_edge->previous().lock()) {
        return std::make_pair(
            std::min(m_edge->target(), prev->target(), cmp),
            std::max(m_edge->target(), prev->target(), cmp));
    }
    throw std::runtime_error("Unconnected edge!");
}

/** All edges between the same two nodes are equal. */
inline bool UndirectedEdge::operator==(const UndirectedEdge &other) const
    noexcept
{
    const auto &lhs = nodes();
    const auto &rhs = other.nodes();

    return lhs.first.lock() == rhs.first.lock()
        && lhs.second.lock() == rhs.second.lock();
}
inline bool UndirectedEdge::operator!=(const UndirectedEdge &other) const
    noexcept
{
    return !(*this == other);
}

/**
 * @returns Referred half-edge reference.
 */
inline const DirectedEdge::pointer_type &UndirectedEdge::referred() const
{
    return m_edge;
}
}  // namespace graph
}  // namespace lod
#endif /* end of include guard: EDGE_H_HRCWE1XD */
