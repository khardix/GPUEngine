/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Graph representation of triangle mesh (implementation).
 */

#include <algorithm>
#include <array>
#include <functional>
#include <tuple>

#include <glm/gtc/type_ptr.hpp>

#include <graph/Mesh.h>

/** Inserts all triangles from original mesh into the graph representation.
 * @param[in] original The mesh to analyze.
 */
lod::graph::Mesh::Mesh(
    /*const*/ ge::sg::Mesh &original)  // FIXME: Iterators not taking const
{
    using namespace std::placeholders;

    auto edge_cache = EdgeCache{};
    auto processor = std::bind(&Mesh::insert, this, _1, std::ref(edge_cache));

    auto begin = ge::sg::MeshPositionIteratorBegin(&original);
    auto end = ge::sg::MeshPositionIteratorEnd(&original);
    std::for_each(begin, end, processor);
}

/** Inserts single triangle into the graph.
 * @param[in] triangle The (raw) triangle to insert.
 * @param[in] cache Cache for fast retrieval of existing edges.
 */
void lod::graph::Mesh::insert(
    const ge::sg::Triangle &triangle, EdgeCache &cache)
{
    using nonstd::get;
    using nonstd::holds_alternative;
    using invalid = DirectedEdge::invalid;

    // insert nodes
    auto nodes = std::array<std::reference_wrapper<const Node>, 3>{
        *(m_nodes.insert(Node{glm::make_vec3(triangle[0])}).first),
        *(m_nodes.insert(Node{glm::make_vec3(triangle[1])}).first),
        *(m_nodes.insert(Node{glm::make_vec3(triangle[2])}).first),
    };

    // prepare edges
    auto edges = std::array<std::unique_ptr<DirectedEdge>, 3>{
        std::make_unique<DirectedEdge>(),
        std::make_unique<DirectedEdge>(),
        std::make_unique<DirectedEdge>(),
    };

    // connect nodes and edges
    for (auto source = std::size_t{0}; source < edges.size(); ++source) {
        auto target = (source + 1) % edges.size();

        // point edges to their adjacent objects
        edges.at(target)->target = &(nodes.at(target).get());
        edges.at(target)->previous = edges.at(source).get();

        // add outgoing edge reference if necessary
        if (nodes.at(source).get().edge == nullptr) {
            nodes.at(source).get().edge = edges.at(target).get();
        }
    }

    // insert edges into the graph
    for (auto &&edge : edges) {
        auto cached = cache.find(UndirectedEdge(*edge));
        if (cached != cache.end()) {
            auto &&opposite = cached->referred();

            // propagate invalid state
            if (holds_alternative<invalid>(opposite.neighbour)) {
                edge->neighbour = opposite.neighbour;
            }
            // detect too many neighbours
            else if (get<DirectedEdge *>(opposite.neighbour) != nullptr) {
                edge->neighbour = opposite.neighbour = invalid::nonmanifold;
            }
            // connect neighbours
            else {
                edge->neighbour = &opposite;
                opposite.neighbour = edge.get();
            }
        }
        else {
            cache.emplace(*edge);
        }

        m_edges.insert(std::exchange(edge, nullptr));
    }
}
