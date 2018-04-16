#include <algorithm>
#include <array>
#include <iostream>
#include <memory>

#include "catch.hpp"
#include <graph/Mesh.h>

SCENARIO(
    "Conversion from ge::sg::Mesh"
    "[graph]")
{
    GIVEN("Simple manifold mesh")
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
        indices->size = static_cast<int>(
            indices_data->size() * sizeof(indices_data->at(0)));
        indices->numComponents = 3;
        indices->type = ge::sg::AttributeDescriptor::DataType::UNSIGNED_INT;
        indices->semantic = ge::sg::AttributeDescriptor::Semantic::indices;
        indices->data = indices_data;

        auto scene_mesh = ge::sg::Mesh{};
        scene_mesh.count = indices_data->size();  // ?!
        scene_mesh.primitive = ge::sg::Mesh::PrimitiveType::TRIANGLES;
        scene_mesh.attributes.push_back(position);
        scene_mesh.attributes.push_back(indices);

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
                auto predicate
                    = [](const Node &node) { return node.edge != nullptr; };

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
                auto compare = [](const std::unique_ptr<DirectedEdge> &edge) {
                    if (!holds_alternative<DirectedEdge *>(edge->neighbour)) {
                        return false;
                    }
                    if (get<DirectedEdge *>(edge->neighbour) == nullptr) {
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
                auto predicate = [](const std::unique_ptr<DirectedEdge> &edge) {
                    return (edge->previous != nullptr)
                        && (edge->previous->previous != nullptr)
                        && (edge->previous->previous->previous == edge.get());
                };

                REQUIRE(std::all_of(begin, end, predicate));
            }
        }
    }
}
