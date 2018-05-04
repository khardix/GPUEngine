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
    const std::array<const Node *, 3> &nodes)
{
    auto result = DetachedTriangle{};
    auto previous = static_cast<DirectedEdge *>(nullptr);

    std::transform(
        std::cbegin(nodes),
        std::cend(nodes),
        std::begin(result),
        [&previous](const auto &node) {
            auto edge = std::make_unique<DirectedEdge>();
            edge->target = node;
            edge->previous = previous;

            previous = edge.get();
            return edge;
        });

    // connect the first and last edge to complete the circle
    result[0]->previous = previous;

    return result;
}
