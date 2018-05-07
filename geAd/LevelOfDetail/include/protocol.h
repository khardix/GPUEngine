#ifndef PROTOCOL_H_DJOVTO94
#define PROTOCOL_H_DJOVTO94
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structures for passing data between parts of simplification algorithm.
 */

#include <functional>
#include <memory>
#include <type_traits>

#include "graph/Edge.h"
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
}  // namespace lod

#endif /* end of include guard: PROTOCOL_H_DJOVTO94 */
