#ifndef PROTOCOL_H_DJOVTO94
#define PROTOCOL_H_DJOVTO94
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structures for passing data between parts of simplification algorithm.
 */

#include <functional>
#include <memory>
#include <type_traits>

#include <geSG/Mesh.h>
#include <glm/gtc/type_ptr.hpp>

#include "graph/Edge.h"
#include "graph/Mesh.h"
#include "graph/Node.h"

namespace lod {
namespace operation {
/// @brief Tag type for operations working with nodes.
struct NodeTag {
    using element_type = graph::Node;
};
/// @brief Tag type for operations working with half-edges.
struct HalfEdgeTag {
    using element_type = std::shared_ptr<graph::DirectedEdge>;
};
/// @brief Tag type for operations working with full-edges.
struct FullEdgeTag {
    using element_type = std::shared_ptr<graph::DirectedEdge>;
};

/// @brief Pointer to the evaluated element.
template <typename Element>
class ElementPointer {
public:
    using type = Element;
    /// @brief For concrete type queries.
    using pointer_type = std::add_pointer_t<type>;
    /// @brief For (ordered) associate container templating.
    using pointer_cmp = std::less<pointer_type>;

    ElementPointer() noexcept = default;
    ElementPointer(type &element) noexcept : m_ptr(std::addressof(element)) {}
    ElementPointer(pointer_type ptr) noexcept : m_ptr(std::move(ptr)) {}

    /// @brief Decay into the stored pointer.
    operator pointer_type() const noexcept { return m_ptr; }

    /// @brief Access the evaluated element.
    type &get() const noexcept;
    /// @brief Determine if the stored pointer is valid.
    bool valid() const noexcept;

private:
    pointer_type m_ptr = nullptr;
};
/// @brief Specialization for shared_ptr as element.
template <typename Element>
class ElementPointer<std::shared_ptr<Element>> {
public:
    using type = std::shared_ptr<Element>;
    /// @brief For concrete type queries.
    using pointer_type = std::weak_ptr<Element>;
    /// @brief For (ordered) associate container templating.
    using pointer_cmp = std::owner_less<pointer_type>;

    ElementPointer() noexcept = default;
    ElementPointer(const type &element) noexcept : m_ptr(element) {}
    ElementPointer(pointer_type ptr) noexcept : m_ptr(std::move(ptr)) {}

    /// @brief Decay into the stored pointer.
    operator pointer_type() const noexcept { return m_ptr; }

    /// @brief Access the evaluated element.
    type get() const noexcept;
    /// @brief Determine if the stored pointer is valid.
    bool valid() const noexcept;
    /// @brief Access the weak reference in this object.
    pointer_type as_weak() const noexcept;

private:
    pointer_type m_ptr;
};

/// @brief Simple operation cost measurement.
template <typename Element, typename ErrorType = float>
class Simple {
public:
    using cost_type = ErrorType;
    using element_pointer = ElementPointer<Element>;

    explicit Simple(element_pointer element, cost_type cost) noexcept;

    /// @brief Access stored element.
    const element_pointer &element() const noexcept;
    /// @brief Access stored cost.
    const cost_type &cost() const noexcept;

    /// @brief Check the poiter validity.
    explicit operator bool() const noexcept;
    /// @brief Compare operations.
    bool operator<(const Simple &other) const noexcept;
    bool operator<=(const Simple &other) const noexcept;
    bool operator>(const Simple &other) const noexcept;
    bool operator>=(const Simple &other) const noexcept;
    bool operator==(const Simple &other) const noexcept;
    bool operator!=(const Simple &other) const noexcept;
    /// @brief Compare cost of operation.
    bool operator<(const ErrorType &cost) const noexcept;

private:
    element_pointer m_element;
    cost_type       m_cost = static_cast<ErrorType>(0);
};

/// @brief Error measurement with placement hint.
template <typename Element, typename ErrorType = float>
class VertexPlacement : public Simple<Element, ErrorType> {
public:
    using position_type = graph::Node::position_type;

    explicit VertexPlacement(
        ElementPointer<Element> element,
        ErrorType               cost,
        position_type           position) noexcept;

    /// @brief Access the stored hint.
    const position_type &position_hint() const noexcept;

private:
    position_type m_hint = {0.f, 0.f, 0.f};
};
}  // namespace operation

/// @brief Mutable state of the simplifaction process
template <typename Element>
class SimplificationState {
public:
    using element_type = typename operation::ElementPointer<Element>::type;
    using pointer_set_type = typename std::unordered_set<element_type>;
    using node_set_type = std::unordered_set<graph::Node::pointer_type>;

    explicit SimplificationState(graph::Mesh &mesh) noexcept;

    /// @brief Access current mesh state.
    graph::Mesh &mesh() noexcept { return m_mesh; }

    /// @brief Access dirty elements.
    pointer_set_type &dirty() noexcept { return m_dirty_elements; }
    /// @brief Mark element as dirty.
    SimplificationState &mark_dirty(element_type dirty_element);

    /// @brief Access deleted nodes.
    node_set_type &deleted_nodes() noexcept { return m_deleted_nodes; }
    /// @brief Mark node as deleted from the mesh.
    SimplificationState &mark_deleted(graph::Node::pointer_type deleted_node);
    /// @brief Mark edge as deleted from the mesh.
    SimplificationState &mark_deleted(
        graph::DirectedEdge::pointer_type deleted_edge);

    /// @brief Export current state of the mesh.
    ge::sg::Mesh export_mesh() const;
    /// @brief Export geomorph attributes.
    const SimplificationState &update_geomorph(ge::sg::Mesh &detailed) const;

private:
    graph::Mesh &m_mesh;  ///< Decimated mesh.
    /// Elements that changed state since last cost evaluation.
    pointer_set_type m_dirty_elements = {};
    /// Vertices already removed from mesh.
    node_set_type m_deleted_nodes = {};
};

// Inline and template members
namespace operation {
template <typename Element>
inline auto ElementPointer<Element>::get() const noexcept -> type &
{
    return *m_ptr;
}

template <typename Element>
inline bool ElementPointer<Element>::valid() const noexcept
{
    return m_ptr != nullptr;
}

template <typename Element>
inline auto ElementPointer<std::shared_ptr<Element>>::get() const noexcept
    -> type
{
    return m_ptr.lock();
}

template <typename Element>
inline bool ElementPointer<std::shared_ptr<Element>>::valid() const noexcept
{
    return !m_ptr.expired();
}

template <typename Element, typename ErrorType>
inline Simple<Element, ErrorType>::Simple(
    ElementPointer<Element> element, ErrorType cost) noexcept
    : m_element(std::move(element)), m_cost(std::move(cost))
{
}

template <typename Element, typename ErrorType>
inline const ElementPointer<Element> &Simple<Element, ErrorType>::element()
    const noexcept
{
    return m_element;
}

template <typename Element, typename ErrorType>
inline const ErrorType &Simple<Element, ErrorType>::cost() const noexcept
{
    return m_cost;
}

template <typename Element, typename ErrorType>
inline Simple<Element, ErrorType>::operator bool() const noexcept
{
    return m_element.valid();
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator<(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return m_cost < other.m_cost;
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator<=(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return operator<(other) || operator==(other);
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator>(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return !operator<=(other);
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator>=(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return !operator<(other);
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator==(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return m_element.get() == other.m_element.get() && m_cost == other.m_cost;
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator!=(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return !operator==(other);
}

template <typename Element, typename ErrorType>
inline bool Simple<Element, ErrorType>::operator<(const ErrorType &cost) const
    noexcept
{
    return m_cost < cost;
}

template <typename Element, typename ErrorType>
inline VertexPlacement<Element, ErrorType>::VertexPlacement(
    ElementPointer<Element> element,
    ErrorType               cost,
    position_type           position) noexcept
    : Simple<Element, ErrorType>(std::move(element), std::move(cost)),
      m_hint(std::move(position))
{
}

template <typename Element, typename ErrorType>
inline auto VertexPlacement<Element, ErrorType>::position_hint() const noexcept
    -> const position_type &
{
    return m_hint;
}
}  // namespace operation

template <typename Element>
inline SimplificationState<Element>::SimplificationState(
    graph::Mesh &mesh) noexcept
    : m_mesh(mesh)
{
}

/** Inserts the element into the set of dirty elements.
 * @param dirty_element The element to insert.
 * @returns Reference to self.
 */
template <typename Element>
inline SimplificationState<Element> &SimplificationState<Element>::mark_dirty(
    element_type dirty_element)
{
    m_dirty_elements.insert(std::move(dirty_element));

    return *this;
}

/** "Deletes" a Node from the mesh.
 * "Deleted" (decimated) nodes are kept in dedicated set in order to be usable
 * for after-decimation calculations. To truly delete a Node from memory, clear
 * the deleted_nodes() container.
 * @param deleted_node The node to remove from the mesh.
 * @return Reference to self.
 */
template <typename Element>
inline SimplificationState<Element> &SimplificationState<Element>::mark_deleted(
    graph::Node::pointer_type deleted_node)
{
    m_mesh.nodes().erase(deleted_node);
    m_deleted_nodes.insert(std::move(deleted_node));
    return *this;
}

/** "Deletes" a node from the mesh and from the dirty elements set.
 * @see SimplificationState<Element>::mark_deleted()
 * @param deleted_node The node to remove from the mesh and dirty elements.
 * @return Reference to self.
 */
template <>
inline SimplificationState<graph::Node::pointer_type>
    &SimplificationState<graph::Node::pointer_type>::mark_deleted(
        graph::Node::pointer_type deleted_node)
{
    m_mesh.nodes().erase(deleted_node);
    m_dirty_elements.erase(deleted_node);
    m_deleted_nodes.insert(std::move(deleted_node));
    return *this;
}

/** Deletes an edge from the mesh.
 * @param deleted_edge The edge to remove from the mesh.
 * @return reference to self.
 */
template <typename Element>
inline SimplificationState<Element> &SimplificationState<Element>::mark_deleted(
    graph::DirectedEdge::pointer_type deleted_edge)
{
    m_mesh.edges().erase(deleted_edge);
    return *this;
}

/** Deletes an edge from the mesh and from the dirty elements set.
 * @param deleted_edge The edge to remove from the mesh and dirty elements.
 * @return reference to self.
 */
template <>
inline SimplificationState<graph::DirectedEdge::pointer_type>
    &SimplificationState<graph::DirectedEdge::pointer_type>::mark_deleted(
        graph::DirectedEdge::pointer_type deleted_edge)
{
    m_mesh.edges().erase(deleted_edge);
    m_dirty_elements.erase(deleted_edge);
    return *this;
}

/** Export current state of the mesh for rendering.
 * @return Renderable equivalent of current mesh state.
 */
template <typename Element>
inline ge::sg::Mesh SimplificationState<Element>::export_mesh() const
{
    return static_cast<ge::sg::Mesh>(m_mesh);
}

/** Fill/update geomorphing attribute of detailed mesh.
 * @param[in,out] detailed The detailed mesh to update.
 * @return Reference to self.
 */
template <typename Element>
const SimplificationState<Element>
    &SimplificationState<Element>::update_geomorph(ge::sg::Mesh &detailed) const
{
    using SM = ge::sg::AttributeDescriptor::Semantic;
    using DT = ge::sg::AttributeDescriptor::DataType;
    using byte = unsigned char;

    auto positions = detailed.getAttribute(SM::position);
    if (positions == nullptr) {
        throw std::runtime_error("Mesh without positions!");
    }

    auto geomorph = detailed.getAttribute(SM::unknown);  // FIXME semantic
    if (geomorph == nullptr) {
        geomorph = std::make_shared<ge::sg::AttributeDescriptor>();
        detailed.attributes.push_back(geomorph);
    }

    auto *const positions_data
        = reinterpret_cast<byte *>(positions->data.get());
    const auto &positions_size = static_cast<std::size_t>(positions->size);
    const auto &positions_step = positions->stride
        ? positions->stride
        : positions->getSize(positions->type) * positions->numComponents;

    geomorph->numComponents = 3;
    geomorph->type = DT::FLOAT;
    geomorph->semantic = SM::unknown;  // FIXME How to add semantic?
    geomorph->size = static_cast<int>(
        (positions_size / positions_step) * geomorph->getSize(geomorph->type)
        * geomorph->numComponents);
    geomorph->data
        = std::make_unique<float[]>(static_cast<std::size_t>(geomorph->size));

    auto *const geomorph_data = reinterpret_cast<byte *>(geomorph->data.get());
    const auto &geomorph_step
        = geomorph->getSize(geomorph->type) * geomorph->numComponents;

    for (auto i = 0u; i < positions_size / positions_step; ++i) {
        auto *current_position = reinterpret_cast<float *>(
            positions_data + positions->offset + i * positions_step);  // NOLINT
        auto *current_geomorph = reinterpret_cast<float *>(
            geomorph_data + i * geomorph_step);  // NOLINT

        auto current_node = graph::Node::make(glm::make_vec3(current_position));

        auto found = m_deleted_nodes.find(current_node);
        if (found != m_deleted_nodes.end()) {
            std::copy_n(
                glm::value_ptr((*found)->geomorph_target()->position()),
                3,
                current_geomorph);
        }
        else {
            std::copy_n(
                glm::value_ptr(current_node->position()), 3, current_geomorph);
        }
    }

    return *this;
}

}  // namespace lod

#endif /* end of include guard: PROTOCOL_H_DJOVTO94 */
