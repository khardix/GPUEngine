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
    auto &C = *(nodes.insert(Node{{0.f, 0.f, 0.f}}).first);
    auto &X = *(nodes.insert(Node{{1.f, 0.f, 0.f}}).first);
    auto &Y = *(nodes.insert(Node{{0.f, 1.f, 0.f}}).first);
    auto &Z = *(nodes.insert(Node{{0.f, 0.f, 1.f}}).first);

    auto CXY = make_triangle({&C, &X, &Y});
    auto CYZ = make_triangle({&C, &Y, &Z});
    auto CZX = make_triangle({&C, &Z, &X});

    // neighbours
    CXY[0]->neighbour() = CYZ[1];
    CYZ[1]->neighbour() = CXY[0];

    CYZ[0]->neighbour() = CZX[1];
    CZX[1]->neighbour() = CYZ[0];

    CZX[0]->neighbour() = CXY[1];
    CXY[1]->neighbour() = CZX[0];

    // emanating edges
    C.edge = CYZ[1];
    X.edge = CZX[0];
    Y.edge = CXY[0];
    Z.edge = CYZ[0];

    auto edges = Mesh::EdgeSet{};
    std::move(CXY.begin(), CXY.end(), std::inserter(edges, edges.end()));
    std::move(CYZ.begin(), CYZ.end(), std::inserter(edges, edges.end()));
    std::move(CZX.begin(), CZX.end(), std::inserter(edges, edges.end()));

    return Mesh(std::move(nodes), std::move(edges));
}

lod::graph::DirectedEdge::pointer_type regular_edge(
    const lod::graph::Mesh &mesh)
{
    const auto center = lod::graph::Node{{0.f, 0.f, 0.f}};
    auto       center_it = mesh.nodes().find(center);
    REQUIRE(center_it != mesh.nodes().end());

    return center_it->edge.lock();
}

lod::graph::DirectedEdge::pointer_type semiborder_edge(
    const lod::graph::Mesh &mesh)
{
    const auto edge = regular_edge(mesh);
    return nonstd::get<lod::graph::DirectedEdge::weak_type>(edge->neighbour())
        .lock();
}

lod::graph::DirectedEdge::pointer_type border_edge(const lod::graph::Mesh &mesh)
{
    const auto X = lod::graph::Node{{1.f, 0.f, 0.f}};
    auto       X_it = mesh.nodes().find(X);
    REQUIRE(X_it != mesh.nodes().end());

    for (auto &&edge : lod::graph::emanating_edges(*X_it)) {
        if (*edge->target() != lod::graph::Node{{0.f, 0.f, 0.f}}) {
            return edge;
        }
    }
    return nullptr;
}

SCENARIO(
    "Application of half-edge collapse operator"
    "[operator]")
{
    using namespace lod;

    using Tag = operation::HalfEdgeTag;
    using HalfEdgeCollapse = oper::EdgeCollapse<Tag>;
    using Operation = operation::Simple<Tag::element_type>;

    GIVEN("Two opposite triangles")
    {
        using lod::oper::common::EdgeCollapse;

        auto O = graph::Node{{0.f, 0.f, 0.f}};
        auto T = graph::Node{{1.f, 0.f, 0.f}};
        auto C = graph::Node{{0.f, 1.f, 0.f}};

        auto A = graph::Node{{0.f, -1.f, 0.f}};
        auto B = graph::Node{{0.f, 1.f, 1.f}};

        auto triangle = graph::make_triangle({&O, &T, &A});

        WHEN("Measuring transformation for folds")
        {
            THEN("Potential fold is detected")
            {
                REQUIRE(EdgeCollapse::would_fold(*triangle[1], C, A));
            }
            THEN("Small change passes")
            {
                REQUIRE(!EdgeCollapse::would_fold(*triangle[1], C, B));
            }
        }
    }

    GIVEN("Mesh and Half-Edge collapse operator")
    {
        auto mesh = make_mesh();
        auto collapse = HalfEdgeCollapse{};

        WHEN("A regular edge is collapsed")
        {
            auto  operation = Operation{regular_edge(mesh), 0.f};
            auto &origin
                = *(operation.element().get()->previous().lock()->target());
            collapse(mesh, operation);

            THEN("The mesh contains expected number of elements")
            {
                REQUIRE(mesh.nodes().size() == 3);
                REQUIRE(mesh.edges().size() == 3);
            }
            THEN("All edges are boundary edges")
            {
                auto is_boundary
                    = [](const auto &edge) { return edge->boundary(); };

                REQUIRE(std::all_of(
                    mesh.edges().cbegin(), mesh.edges().cend(), is_boundary));
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
                    return mesh.edges().count(node.edge.lock()) > 0;
                };

                for (const auto &node : mesh.nodes()) {
                    CAPTURE(node.position);
                    REQUIRE(valid_ref(node));
                }
            }
            THEN("All edges point to valid nodes")
            {
                auto valid_target = [&mesh](const auto &edge) {
                    auto target_ptr = edge->target();
                    auto mesh_it = mesh.nodes().find(*target_ptr);
                    if (mesh_it == mesh.nodes().end()) {
                        FAIL("Node is not known to mesh");
                        return false;
                    }
                    auto mesh_ptr = &*mesh_it;

                    return target_ptr == mesh_ptr;
                };

                REQUIRE(std::all_of(
                    mesh.edges().begin(), mesh.edges().end(), valid_target));
            }
        }

        WHEN("A semi-boundary edge is collapsed")
        {
            auto operation = Operation{semiborder_edge(mesh), 0.f};
            auto modified = collapse(mesh, operation);

            THEN("The mesh is not modified")
            {
                REQUIRE(mesh.nodes().size() == 4);
                REQUIRE(mesh.edges().size() == 9);
                REQUIRE(modified.empty());
            }
        }

        WHEN("A boundary edge is collapsed")
        {
            auto operation = Operation{border_edge(mesh), 0.f};
            auto modified = collapse(mesh, operation);

            THEN("The mesh is not modified")
            {
                REQUIRE(mesh.nodes().size() == 4);
                REQUIRE(mesh.edges().size() == 9);
                REQUIRE(modified.empty());
            }
        }
    }
}
