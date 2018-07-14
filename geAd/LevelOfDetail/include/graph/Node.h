#ifndef NODE_H_Y6XJRGFA
#define NODE_H_Y6XJRGFA
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesh graph component: Node.
 */

#include <functional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace lod {
namespace graph {
struct DirectedEdge;

/// @brief Single vertex with adjacency information.
class Node : public std::enable_shared_from_this<Node> {
public:
    /// @brief Owning reference to a Node.
    using pointer_type = std::shared_ptr<Node>;
    using const_pointer_type = std::shared_ptr<const Node>;
    /// @brief Non-owning pointer to a Node.
    using weak_type = std::weak_ptr<Node>;
    using const_weak_type = std::weak_ptr<const Node>;

    /// @brief Position in the object space.
    using position_type = glm::vec3;
    /// @brief Non-owning reference to outgoing edge.
    using edge_pointer_type = std::weak_ptr<DirectedEdge>;

    /// @brief Enforce shared pointer creation.
    template <typename... Args>
    static pointer_type make(Args &&... args);
    /// @brief "Cast" the edge to shared pointer.
    pointer_type       as_shared() { return shared_from_this(); }
    const_pointer_type as_shared() const { return shared_from_this(); }
    /// @brief "Cast" the edge to weak pointer.
    weak_type       as_weak() { return shared_from_this(); }
    const_weak_type as_weak() const { return shared_from_this(); }

    /// @brief Access stored position.
    const position_type &position() const noexcept { return m_position; }
    /// @brief Access stored edge pointer.
    edge_pointer_type &      edge() noexcept { return m_edge; }
    const edge_pointer_type &edge() const noexcept { return m_edge; }

    /// @brief Compare two Nodes.
    bool operator==(const Node &other) const noexcept;
    bool operator!=(const Node &other) const noexcept;

protected:
    explicit Node(
        position_type     position = {0.f, 0.f, 0.f},
        edge_pointer_type edge = {}) noexcept
        : m_position{std::move_if_noexcept(position)},
          m_edge{std::move_if_noexcept(edge)}
    {
    }

private:
    /// Vertex position in model space.
    const position_type m_position = {0.f, 0.f, 0.f};
    edge_pointer_type   m_edge = {};  ///< Arbitrary first outgoing edge.
};
}  // namespace graph
}  // namespace lod


// Inline and template members
namespace std {
/** Existing Node is hashed by its position.
 * Non-existent is hashed as nullptr.
 */
template <>
struct hash<lod::graph::Node::pointer_type> {
    using argument_type = lod::graph::Node::pointer_type;
    using position_type = lod::graph::Node::position_type;

    std::size_t operator()(const argument_type &node) const noexcept
    {
        if (node == nullptr) {
            return std::hash<argument_type::element_type *>{}(nullptr);
        }
        else {
            return std::hash<position_type>{}(node->position());
        }
    }
};
/** Node pointers are considered equal (for sorting in containers)
 * if they point to Nodes with the same position.
 */
template <>
struct equal_to<lod::graph::Node::pointer_type> {
    using argument_type = lod::graph::Node::pointer_type;

    bool operator()(const argument_type &lhs, const argument_type &rhs) const
    {
        if (lhs == rhs) {
            return true;
        }
        if (lhs == nullptr || rhs == nullptr) {
            return false;
        }
        return *lhs == *rhs;
    }
};
}  // namespace std

namespace lod {
namespace graph {
/** Create Node at heap behind a shared pointer.
 * Parameters are passed directly to the constructor.
 */
template <typename... Args>
inline auto Node::make(Args &&... args) -> pointer_type
{
    return pointer_type{new Node(std::forward<Args>(args)...)};
}

/// Nodes are considered equal if they are at the same position.
inline bool Node::operator==(const Node &other) const noexcept
{
    return glm::all(glm::equal(m_position, other.m_position));
}
/// @see operator==()
inline bool Node::operator!=(const Node &other) const noexcept
{
    return !(*this == other);
}
}  // namespace graph
}  // namespace lod

#endif /* end of include guard: NODE_H_Y6XJRGFA */
