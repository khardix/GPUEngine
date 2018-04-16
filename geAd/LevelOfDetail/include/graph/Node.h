#ifndef NODE_H_Y6XJRGFA
#define NODE_H_Y6XJRGFA
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesh graph component: Node.
 */

#include <functional>

#include <glm/glm.hpp>

#include "../util/hash_combinator.h"

namespace lod {
namespace graph {
struct DirectedEdge;

/// @brief Single vertex with adjacency information.
struct Node {
    friend struct std::hash<Node>;

    glm::vec3 position = {0.f, 0.f, 0.f};  ///< Vertex position in model space.
    mutable DirectedEdge *edge = nullptr;  ///< Arbitrary first outgoing edge.

    bool operator==(const Node &other) const noexcept;
    bool operator!=(const Node &other) const noexcept;
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
        return lod::util::hash_combinator(
            0, node.position.x, node.position.y, node.position.z);
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
