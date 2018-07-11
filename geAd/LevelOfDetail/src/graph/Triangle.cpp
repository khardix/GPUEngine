/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Mesh graph component: Triangle (implementation).
 */

#include <algorithm>

#include <graph/Triangle.h>

/**
 * @param[in] nodes The nodes in edge order.
 * @returns Detached triangle.
 */
lod::graph::DetachedTriangle lod::graph::make_triangle(
    const std::array<const Node::const_weak_type, 3> &nodes)
{
    auto result = DetachedTriangle{};
    auto previous = DirectedEdge::pointer_type(nullptr);

    std::transform(
        std::cbegin(nodes),
        std::cend(nodes),
        std::begin(result),
        [&previous](const auto &node) {
            auto edge = DirectedEdge::make(node, previous);
            previous = edge;
            return edge;
        });

    // connect the first and last edge to complete the circle
    result[0]->previous() = previous;

    return result;
}
