#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <vector>

#include "catch.hpp"
#include <graph/Mesh.h>

// conversions
template <typename T>
std::vector<T> read_attribute(const ge::sg::AttributeDescriptor &attr)
{
    const auto data = std::static_pointer_cast<T>(attr.data);
    const auto size = attr.size / sizeof(T);
    return std::vector<T>(data.get(), data.get() + size);  // NOLINT
}

// factory functions
ge::sg::Mesh make_scene_mesh()
{
    using PositionArray = std::array<float, 4 * 3>;
    using IndexArray = std::array<unsigned, 4 * 3>;

    // clang-format off
        auto position_data = std::make_shared<PositionArray>(PositionArray{ 
            0.f, 0.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f, 1.f,
        });
        auto indices_data = std::make_shared<IndexArray>(IndexArray{
            0, 1, 3,
            0, 2, 1,
            0, 3, 2,
            1, 2, 3,
        });
    // clang-format on

    auto position = std::make_shared<ge::sg::AttributeDescriptor>();
    position->size = static_cast<int>(
        position_data->size() * sizeof(position_data->at(0)));
    position->numComponents = 3;
    position->type = ge::sg::AttributeDescriptor::DataType::FLOAT;
    position->semantic = ge::sg::AttributeDescriptor::Semantic::position;
    position->data = position_data;

    auto indices = std::make_shared<ge::sg::AttributeDescriptor>();
    indices->size
        = static_cast<int>(indices_data->size() * sizeof(indices_data->at(0)));
    indices->numComponents = 3;
    indices->type = ge::sg::AttributeDescriptor::DataType::UNSIGNED_INT;
    indices->semantic = ge::sg::AttributeDescriptor::Semantic::indices;
    indices->data = indices_data;

    auto scene_mesh = ge::sg::Mesh{};
    scene_mesh.count = indices_data->size();  // ?!
    scene_mesh.primitive = ge::sg::Mesh::PrimitiveType::TRIANGLES;
    scene_mesh.attributes.push_back(position);
    scene_mesh.attributes.push_back(indices);

    return scene_mesh;
}

SCENARIO(
    "Conversion from ge::sg::Mesh"
    "[graph]")
{
    GIVEN("Simple manifold mesh")
    {
        auto scene_mesh = make_scene_mesh();

        WHEN("The mesh is converted")
        {
            using namespace lod::graph;
            using namespace nonstd;

            Mesh graph_mesh(scene_mesh);

            THEN("The vertex count is the same")
            {
                REQUIRE(graph_mesh.nodes().size() == scene_mesh.count / 3);
            }

            THEN("All nodes have emanating edges")
            {
                auto begin = std::cbegin(graph_mesh.nodes());
                auto end = std::cend(graph_mesh.nodes());
                auto predicate = [](const Mesh::NodeSet::value_type &node) {
                    return !node.edge.expired();
                };

                REQUIRE(std::all_of(begin, end, predicate));
            }

            THEN("There is expected number of edges")
            {
                REQUIRE(graph_mesh.edges().size() == 12);
            }

            THEN("All edges have neighbours")
            {
                auto begin = graph_mesh.edges().cbegin();
                auto end = graph_mesh.edges().cend();
                auto compare = [](const Mesh::EdgeSet::value_type &edge) {
                    if (!holds_alternative<DirectedEdge::weak_type>(
                            edge->neighbour())) {
                        return false;
                    }
                    if (get<DirectedEdge::weak_type>(edge->neighbour())
                            .expired()) {
                        return false;
                    }

                    return true;
                };

                REQUIRE(std::all_of(begin, end, compare));
            }

            THEN("All edges are part of a triangle")
            {
                auto begin = std::cbegin(graph_mesh.edges());
                auto end = std::cend(graph_mesh.edges());
                auto predicate = [](const Mesh::EdgeSet::value_type &edge) {
                    return (!edge->previous().expired())
                        && (!edge->previous().lock()->previous().expired())
                        && (edge->previous()
                                .lock()
                                ->previous()
                                .lock()
                                ->previous()
                                .lock()
                            == edge);
                };

                REQUIRE(std::all_of(begin, end, predicate));
            }
        }
    }
}

SCENARIO(
    "Roundtrip mesh conversion"
    "[graph]")
{
    GIVEN("Simple manifold mesh and its graph equivalent")
    {
        auto        scene_mesh = make_scene_mesh();
        const auto &graph_mesh = lod::graph::Mesh(scene_mesh);

        WHEN("The graph is converted back")
        {
            using SM = ge::sg::AttributeDescriptor::Semantic;

            auto result_mesh = static_cast<ge::sg::Mesh>(graph_mesh);

            THEN("The number of vertices is the same")
            {
                REQUIRE(scene_mesh.count == result_mesh.count);
            }

            THEN("The attributes contain the same amount data")
            {
                for (const auto &semantic : {SM::position, SM::indices}) {
                    CAPTURE(semantic);
                    const auto attr_in = scene_mesh.getAttribute(semantic);
                    const auto attr_out = result_mesh.getAttribute(semantic);
                    REQUIRE(static_cast<bool>(attr_in));
                    REQUIRE(static_cast<bool>(attr_out));

                    REQUIRE(attr_in->size == attr_out->size);
                }
            }

            THEN("Manual confirmation of position values")
            {
                for (auto &&mesh : {&scene_mesh, &result_mesh}) {
                    auto positions = read_attribute<float>(
                        *(mesh->getAttribute(SM::position)));
                    auto indices = read_attribute<unsigned>(
                        *(mesh->getAttribute(SM::indices)));

                    CAPTURE(positions);
                    CAPTURE(indices);

                    REQUIRE((!positions.empty() && !indices.empty()));
                }
            }
        }
    }
}
