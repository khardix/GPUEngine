#ifndef MESHGRAPH_H_PB5REYZQ
#define MESHGRAPH_H_PB5REYZQ

/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Graph representation of triangle mesh.
 */

#include <utility>
#include <vector>

#ifndef variant_CONFIG_MAX_ALIGN_HACK
#define variant_CONFIG_MAX_ALIGN_HACK 1
#endif  // variant_CONFIG_MAX_ALIGN_HACK
// ^ silence variant warning, should have no effect on c++11 and later
#include <nonstd/variant.hpp>

#include <glm/vec3.hpp>

#include "util/hash_combinator.h"

namespace lod {
namespace graph {
struct Node;           ///< @brief Single vertex with adjacency information.
struct DirectedEdge;   ///< @brief Half-edge with adjacency information.
class UndirectedEdge;  ///< @brief Hashable canonical representation of an edge.

struct Node {
    glm::vec3 position = {0.f, 0.f, 0.f};  ///< Vertex position in model space.
    DirectedEdge *edge = nullptr;          ///< Arbitrary first outgoing edge.
};

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

class UndirectedEdge : public std::pair<Node *, Node *> {
public:
    friend struct std::hash<UndirectedEdge>;
    explicit UndirectedEdge(Node *lhs, Node *rhs) noexcept;
};
}  // namespace graph
}  // namespace lod


/* Inline and template members. {{{ */
namespace std {
/// @brief Injected hash functor for undirected edges.
template <>
struct hash<lod::graph::UndirectedEdge> {
    using argument_type = lod::graph::UndirectedEdge;
    using return_type = std::size_t;

    /** @brief Calculate edge hash.
     * The hash should be comutative: `hash(AB) == hash(BA)`.
     */
    return_type operator()(const argument_type &edge) const noexcept
    {
        return lod::util::hash_combinator(0, edge.first, edge.second);
    }
};
}  // namespace std

/** @brief Force consistent ordering of the pointers.
 * @param[in] lhs One terminal node.
 * @param[in] rhs Other terminal node.
 */
inline lod::graph::UndirectedEdge::UndirectedEdge(Node *lhs, Node *rhs) noexcept
    : pair(std::min(lhs, rhs), std::max(lhs, rhs))
{
}
/* }}} Inline and template members. */

#endif /* end of include guard: MESHGRAPH_H_PB5REYZQ */
