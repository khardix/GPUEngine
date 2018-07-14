#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <tuple>

#include <glm/gtx/io.hpp>

#include "catch.hpp"
#include <graph/Mesh.h>
#include <graph/Triangle.h>
#include <graph/algorithm.h>
#include <operator/edge_collapse.h>
#include <util/set_operations.h>

lod::graph::Mesh make_mesh()
{
    using namespace lod::graph;

    auto  nodes = Mesh::NodeSet{};
    auto &C = *(nodes.insert(Node::make(glm::vec3{0.f, 0.f, 0.f})).first);
    auto &X = *(nodes.insert(Node::make(glm::vec3{1.f, 0.f, 0.f})).first);
    auto &Y = *(nodes.insert(Node::make(glm::vec3{0.f, 1.f, 0.f})).first);
    auto &Z = *(nodes.insert(Node::make(glm::vec3{0.f, 0.f, 1.f})).first);
    auto &W = *(nodes.insert(Node::make(glm::vec3{2.f, 0.f, 0.f})).first);

    auto CXY = make_triangle({C, X, Y});
    auto CYZ = make_triangle({C, Y, Z});
    auto CZX = make_triangle({C, Z, X});
    auto XWY = make_triangle({X, W, Y});
    auto XZW = make_triangle({X, Z, W});

    // neighbours
    CXY[0]->neighbour() = CYZ[1];
    CYZ[1]->neighbour() = CXY[0];

    CYZ[0]->neighbour() = CZX[1];
    CZX[1]->neighbour() = CYZ[0];

    CZX[0]->neighbour() = CXY[1];
    CXY[1]->neighbour() = CZX[0];

    XWY[0]->neighbour() = CXY[2];
    CXY[2]->neighbour() = XWY[0];

    XZW[1]->neighbour() = CZX[2];
    CZX[2]->neighbour() = XZW[1];

    XWY[1]->neighbour() = XZW[0];
    XZW[0]->neighbour() = XWY[1];

    // emanating edges
    C->edge() = CYZ[1];
    X->edge() = CZX[0];
    Y->edge() = CXY[0];
    Z->edge() = CYZ[0];
    W->edge() = XZW[0];

    auto edges = Mesh::EdgeSet{};
    std::move(CXY.begin(), CXY.end(), std::inserter(edges, edges.end()));
    std::move(CYZ.begin(), CYZ.end(), std::inserter(edges, edges.end()));
    std::move(CZX.begin(), CZX.end(), std::inserter(edges, edges.end()));
    std::move(XWY.begin(), XWY.end(), std::inserter(edges, edges.end()));
    std::move(XZW.begin(), XZW.end(), std::inserter(edges, edges.end()));

    return Mesh(std::move(nodes), std::move(edges));
}

lod::graph::DirectedEdge::pointer_type regular_edge(
    const lod::graph::Mesh &mesh)
{
    const auto X = lod::graph::Node::make(glm::vec3{1.f, 0.f, 0.f});
    auto       X_it = mesh.nodes().find(X);
    REQUIRE(X_it != mesh.nodes().end());

    return (*X_it)->edge().lock();
}

lod::graph::DirectedEdge::pointer_type semiborder_edge(
    const lod::graph::Mesh &mesh)
{
    const auto Y = lod::graph::Node::make(glm::vec3{0.f, 1.f, 0.f});
    auto       Y_it = mesh.nodes().find(Y);
    REQUIRE(Y_it != mesh.nodes().end());

    return (*Y_it)->edge().lock();
}

lod::graph::DirectedEdge::pointer_type border_edge(const lod::graph::Mesh &mesh)
{
    const auto Y = lod::graph::Node::make(glm::vec3{0.f, 1.f, 0.f});
    auto       Y_it = mesh.nodes().find(Y);
    REQUIRE(Y_it != mesh.nodes().end());

    for (auto &&edge : lod::graph::emanating_edges(**Y_it)) {
        if (edge->boundary()) {
            return edge;
        }
    }
    return nullptr;
}

SCENARIO(
    "Application of edge collapse operators"
    "[operator]")
{
    using namespace lod;

    using Half = operation::HalfEdgeTag;
    using HalfEdgeCollapse = oper::EdgeCollapse<Half>;
    using HalfOperation = operation::Simple<Half::element_type>;

    using Full = operation::FullEdgeTag;
    using FullEdgeCollapse = oper::EdgeCollapse<Full>;
    using FullOperation = operation::VertexPlacement<Full::element_type>;

    GIVEN("Two opposite triangles")
    {
        using lod::oper::common::EdgeCollapse;

        auto O = graph::Node::make(glm::vec3{0.f, 0.f, 0.f});
        auto T = graph::Node::make(glm::vec3{1.f, 0.f, 0.f});
        auto C = graph::Node::make(glm::vec3{0.f, 1.f, 0.f});

        auto A = graph::Node::make(glm::vec3{0.f, -1.f, 0.f});
        auto B = graph::Node::make(glm::vec3{0.f, 1.f, 1.f});

        auto triangle = graph::make_triangle({O, T, A});

        WHEN("Measuring transformation for folds")
        {
            THEN("Potential fold is detected")
            {
                REQUIRE(EdgeCollapse::would_fold(*triangle[1], *C, *A));
            }
            THEN("Small change passes")
            {
                REQUIRE(!EdgeCollapse::would_fold(*triangle[1], *C, *B));
            }
        }
    }

    GIVEN("Mesh and Half-Edge collapse operator")
    {
        auto mesh = make_mesh();
        auto state = lod::SimplificationState<Half::element_type>(mesh);
        auto collapse = HalfEdgeCollapse{};

        WHEN("A regular edge is collapsed")
        {
            auto operation = HalfOperation{regular_edge(mesh), 0.f};
            auto origin = std::const_pointer_cast<lod::graph::Node>(
                operation.element().get()->previous().lock()->target().lock());
            collapse(state, operation);

            THEN("The mesh contains expected number of elements")
            {
                REQUIRE(mesh.nodes().size() == 4);
                REQUIRE(mesh.edges().size() == 9);
            }
            THEN("All edges are connected")
            {
                auto connected = [&mesh](const auto &edge) {
                    auto cnt = mesh.edges().count(edge->previous().lock());
                    return edge->previous().lock() != nullptr && cnt > 0;
                };

                REQUIRE(std::all_of(
                    mesh.edges().cbegin(), mesh.edges().cend(), connected));
            }
            THEN("Mesh does not contain the origin node")
            {
                REQUIRE(mesh.nodes().count(origin) == 0);
            }
            THEN("All nodes have valid edge reference")
            {
                auto valid_ref = [&mesh](const auto &node) {
                    return mesh.edges().count(node->edge().lock()) > 0;
                };

                for (const auto &node : mesh.nodes()) {
                    CAPTURE(node->position());
                    REQUIRE(valid_ref(node));
                }
            }
            THEN("All edges point to valid nodes")
            {
                auto valid_target = [&mesh](const auto &edge) {
                    auto target = std::const_pointer_cast<lod::graph::Node>(
                        edge->target().lock());
                    auto mesh_it = mesh.nodes().find(target);
                    if (mesh_it == mesh.nodes().end()) {
                        FAIL("Node is not known to mesh");
                        return false;
                    }
                    auto &mesh_reference = *mesh_it;

                    return target == mesh_reference;
                };

                REQUIRE(std::all_of(
                    mesh.edges().begin(), mesh.edges().end(), valid_target));
            }
        }

        WHEN("A semi-boundary edge is collapsed")
        {
            auto operation = HalfOperation{semiborder_edge(mesh), 0.f};
            collapse(state, operation);
            auto &modified = state.dirty();

            THEN("The mesh is not modified")
            {
                REQUIRE(mesh.nodes().size() == 5);
                REQUIRE(mesh.edges().size() == 15);
                REQUIRE(modified.empty());
            }
        }

        WHEN("A boundary edge is collapsed")
        {
            auto operation = HalfOperation{border_edge(mesh), 0.f};
            collapse(state, operation);
            auto &modified = state.dirty();

            THEN("The mesh is not modified")
            {
                REQUIRE(mesh.nodes().size() == 5);
                REQUIRE(mesh.edges().size() == 15);
                REQUIRE(modified.empty());
            }
        }
    }

    GIVEN("Mesh and Full-Edge collapse operator")
    {
        auto mesh = make_mesh();
        auto collapse = FullEdgeCollapse{};
        auto state = lod::SimplificationState<Full::element_type>(mesh);

        WHEN("A regular edge is collapsed")
        {
            auto hinted = graph::Node::make(glm::vec3{0.0f, 0.f, 0.f});
            auto collapsed = regular_edge(mesh);
            auto original = std::make_pair(
                std::const_pointer_cast<graph::Node>(
                    collapsed->target().lock()),
                std::const_pointer_cast<graph::Node>(
                    collapsed->previous().lock()->target().lock()));
            auto operation = FullOperation{collapsed, 0.f, hinted->position()};
            collapse(state, operation);

            THEN("The mesh contains expected number of elements")
            {
                REQUIRE(mesh.nodes().size() == 4);
                REQUIRE(mesh.edges().size() == 9);
            }
            THEN("All edges are connected")
            {
                auto connected = [&mesh](const auto &edge) {
                    auto cnt = mesh.edges().count(edge->previous().lock());
                    return edge->previous().lock() != nullptr && cnt > 0;
                };

                REQUIRE(std::all_of(
                    mesh.edges().cbegin(), mesh.edges().cend(), connected));
            }
            THEN("Mesh contains the new node")
            {
                REQUIRE(mesh.nodes().count(hinted) > 0);
            }
            THEN("Mesh does not contain the original nodes")
            {
                if (*hinted != *original.first) {
                    REQUIRE(mesh.nodes().count(original.first) == 0);
                }
                if (*hinted != *original.second) {
                    REQUIRE(mesh.nodes().count(original.second) == 0);
                }
            }
            THEN("All nodes have valid edge reference")
            {
                auto valid_ref = [&mesh](const auto &node) {
                    return mesh.edges().count(node->edge().lock()) > 0;
                };

                for (const auto &node : mesh.nodes()) {
                    CAPTURE(node->position());
                    REQUIRE(valid_ref(node));
                }
            }
            THEN("All edges point to valid nodes")
            {
                auto valid_target = [&mesh](const auto &edge) {
                    auto target = std::const_pointer_cast<graph::Node>(
                        edge->target().lock());
                    auto mesh_it = mesh.nodes().find(target);
                    if (mesh_it == mesh.nodes().end()) {
                        FAIL("Node is not known to mesh");
                        return false;
                    }
                    auto &mesh_reference = *mesh_it;

                    return target == mesh_reference;
                };

                REQUIRE(std::all_of(
                    mesh.edges().begin(), mesh.edges().end(), valid_target));
            }
        }

        WHEN("A semi-boundary edge is collapsed")
        {
            auto operation
                = FullOperation{semiborder_edge(mesh), 0.f, {0.f, 0.f, 0.f}};
            collapse(state, operation);
            auto &modified = state.dirty();

            THEN("The mesh is not modified")
            {
                REQUIRE(mesh.nodes().size() == 5);
                REQUIRE(mesh.edges().size() == 15);
                REQUIRE(modified.empty());
            }
        }

        WHEN("A boundary edge is collapsed")
        {
            auto operation
                = FullOperation{semiborder_edge(mesh), 0.f, {0.f, 0.f, 0.f}};
            collapse(state, operation);
            auto &modified = state.dirty();

            THEN("The mesh is not modified")
            {
                REQUIRE(mesh.nodes().size() == 5);
                REQUIRE(mesh.edges().size() == 15);
                REQUIRE(modified.empty());
            }
        }
    }
}
