/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Quadric Error Metric (QEM) (implementation).
 */

#include <algorithm>
#include <array>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>

#include <glm/gtc/matrix_access.hpp>

#include <graph/algorithm.h>
#include <metric/qem.h>

/** Calculate planar vector ($[a, b, c, d]$, $ax + by + cz + d = 0$).
 * @param[in] triangle Defining triangle of the plane.
 * @returns Planar vector.
 */
const glm::vec4 lod::metric::QEM::plane(const lod::graph::Triangle &triangle)
{
    auto node_positions = std::array<const glm::vec3 *, 3>{};
    std::transform(
        std::cbegin(triangle),
        std::cend(triangle),
        std::begin(node_positions),
        [](const auto &edge) { return &(edge->target->position); });

    const auto &coeff = glm::cross(
        *std::get<1>(node_positions) - *std::get<0>(node_positions),
        *std::get<2>(node_positions) - *std::get<0>(node_positions));
    const auto &d = -glm::dot(coeff, *std::get<0>(node_positions));
    const auto &len = glm::length(coeff);

    return glm::vec4(coeff / len, d / len);
}

/** Provides a quadric for a given Node.
 * @param[in] node The node to evaluate.
 * @returns Quadric for the node; calculated if necessary, cached one if
 * possible.
 */
const glm::mat4 &lod::metric::QEM::quadric(const lod::graph::Node &node) const
{
    if (m_vertex_cache.count(node.position) != 0) {
        return m_vertex_cache.at(node.position);
    }

    const auto triangles = graph::adjacent_triangles(node);
    auto       planes = std::vector<glm::mat4>{};
    planes.reserve(triangles.size());

    std::transform(
        std::cbegin(triangles),
        std::cend(triangles),
        std::back_inserter(planes),
        [](const auto &triangle) {
            const auto p = plane(triangle);
            return glm::outerProduct(p, p);
        });

    return m_vertex_cache[node.position] = std::accumulate(
               std::cbegin(planes), std::cend(planes), glm::mat4(0.f));
}

/** Calculates optimal node position given a quadric.
 * @param[in] quadric The quadric to extract the position from.
 * @returns Optimal new node position.
 */
glm::vec3 lod::metric::QEM::position(const glm::mat4 &quadric)
{
    const auto position_mat = glm::row(quadric, 3, {0.f, 0.f, 0.f, 1.f});
    return static_cast<glm::vec3>(
        glm::normalize(position_mat * glm::vec4{0.f, 0.f, 0.f, 1.f}));
}

/** Calculates the cost of replacing an edge with given quadric with node at
 * given position.
 * @param[in] quadric A quadric belonging to the collapsed edge.
 * @param[in] position Position of a node which will replace the edge.
 * @return The cost of collapse.
 */
lod::metric::QEM::Result::cost_type lod::metric::QEM::error(
    const glm::mat4 &quadric, const glm::vec3 &position)
{
    const auto position_norm = glm::vec4(position, 1.f);
    return glm::dot(position_norm * quadric, position_norm);
}

/** Calculate the collapse cost and provide optimal new node placement.
 * @param[in] edge The edge to evaluate.
 * @returns Cost and placement suggestion.
 */
lod::metric::QEM::Result lod::metric::QEM::operator()(const Element &edge) const
{
    // calculate quadric of the new node: Q = Q1 + Q2
    const auto &nodes = std::make_pair(edge->target, edge->previous->target);
    auto        quadric
        = QEM::quadric(*std::get<0>(nodes)) + QEM::quadric(*std::get<1>(nodes));

    const auto pos = position(quadric);
    const auto err = error(quadric, pos);

    m_vertex_cache.emplace(pos, std::move(quadric));
    return Result(edge, err, pos);
}
