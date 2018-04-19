/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Graph representation of triangle mesh (implementation).
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iterator>
#include <tuple>
#include <unordered_map>

#include <glm/gtc/type_ptr.hpp>

#include <geSG/AttributeDescriptor.h>
#include <geSG/Mesh.h>
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

/** Prepares a position AttributeDescriptor for mesh export.
 * @param node_count Number of nodes to be exported.
 * @returns Shared pointer to the attribute descriptor.
 */
std::shared_ptr<ge::sg::AttributeDescriptor> prepare_positions(
    std::size_t node_count)
{
    using DT = ge::sg::AttributeDescriptor::DataType;
    using SM = ge::sg::AttributeDescriptor::Semantic;

    auto attribute = std::make_shared<ge::sg::AttributeDescriptor>();
    attribute->semantic = SM::position;
    attribute->type = DT::FLOAT;
    attribute->stride = attribute->offset = 0u;
    attribute->numComponents = 3;  // per vertex

    const auto component_count = node_count * attribute->numComponents;
    attribute->size = static_cast<int>(sizeof(float) * component_count);
    attribute->data = std::shared_ptr<float>(
        new float[component_count], std::default_delete<float[]>{});

    return attribute;
}

/** Prepares an index/element AttributeDescriptor for mesh export.
 * @param edge_count Number of edges to be exported.
 * @returns Shared pointer to the attribute descriptor.
 */
std::shared_ptr<ge::sg::AttributeDescriptor> prepare_indices(
    std::size_t edge_count)
{
    using DT = ge::sg::AttributeDescriptor::DataType;
    using SM = ge::sg::AttributeDescriptor::Semantic;

    auto attribute = std::make_shared<ge::sg::AttributeDescriptor>();
    attribute->semantic = SM::indices;
    attribute->type = DT::UNSIGNED_INT;
    attribute->stride = attribute->offset = 0u;
    attribute->numComponents = 1;  // per element

    const auto component_count = edge_count * attribute->numComponents;
    attribute->size = static_cast<int>(sizeof(unsigned) * component_count);
    attribute->data = std::shared_ptr<unsigned>(
        new unsigned[component_count], std::default_delete<unsigned[]>{});

    return attribute;
}

/** Converts current state of the mesh into scene graph representation.
 * @returns Converted mesh.
 * @throws std::runtime_error Error in memory handling.
 */
lod::graph::Mesh::operator ge::sg::Mesh() const
{
    auto index_map = std::unordered_map<const Node *, unsigned>{};
    auto edge_set = std::unordered_set<const DirectedEdge *>{};

    auto result = ge::sg::Mesh();
    result.primitive = ge::sg::Mesh::PrimitiveType::TRIANGLES;
    result.count = static_cast<int>(m_edges.size());  // size of element buffer
    result.attributes.reserve(2);

    // copy positions
    auto  positions = prepare_positions(m_nodes.size());
    auto *raw_pos = std::static_pointer_cast<float>(positions->data).get();
    assert(raw_pos != nullptr);  // NOLINT

    const auto start_pos = raw_pos;
    for (const auto &node : m_nodes) {
        // save index
        index_map[&node]
            = std::distance(start_pos, raw_pos) / positions->numComponents;
        // copy position data
        raw_pos = std::copy_n(
            glm::value_ptr(node.position), positions->numComponents, raw_pos);
    }
    result.attributes.push_back(positions);

    // copy node indexes
    auto  indices = prepare_indices(m_edges.size());
    auto *raw_idx = std::static_pointer_cast<unsigned>(indices->data).get();
    assert(raw_idx != nullptr);  // NOLINT

    for (const auto &edge : m_edges) {
        if (edge_set.count(edge.get()) != 0) {  // skip processed edges
            continue;
        }
        for (const auto &raw_edge : edge->triangle_edges()) {
            *(raw_idx++) = index_map.at(raw_edge->target);  // NOLINT
            edge_set.insert(raw_edge);
        }
    }
    result.attributes.push_back(indices);

    return result;
}
