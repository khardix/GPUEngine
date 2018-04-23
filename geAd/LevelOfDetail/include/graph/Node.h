#ifndef NODE_H_Y6XJRGFA
#define NODE_H_Y6XJRGFA
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesh graph component: Node.
 */

#include <functional>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace lod {
namespace graph {
struct DirectedEdge;

/// @brief Single vertex with adjacency information.
struct Node {
    friend struct std::hash<Node>;
    using position_type = glm::vec3;

    bool operator==(const Node &other) const noexcept;
    bool operator!=(const Node &other) const noexcept;

    /// Vertex position in model space.
    position_type         position = {0.f, 0.f, 0.f};
    mutable DirectedEdge *edge = nullptr;  ///< Arbitrary first outgoing edge.
};
}  // namespace graph
}  // namespace lod


// Inline and template members

namespace std {
template <>
struct hash<lod::graph::Node> {
    using argument_type = lod::graph::Node;
    using return_type = std::size_t;

    /// @brief Node is hashed by its position.
    return_type operator()(const argument_type &node) const noexcept
    {
        return std::hash<argument_type::position_type>{}(node.position);
    }
};
}  // namespace std

/// @brief Nodes are considered equal if they are at the same position.
inline bool lod::graph::Node::operator==(const Node &other) const noexcept
{
    return glm::all(glm::equal(position, other.position));
}
inline bool lod::graph::Node::operator!=(const Node &other) const noexcept
{
    return !(*this == other);
}

#endif /* end of include guard: NODE_H_Y6XJRGFA */
