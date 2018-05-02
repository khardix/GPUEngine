#ifndef PROTOCOL_H_DJOVTO94
#define PROTOCOL_H_DJOVTO94
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structures for passing data between parts of simplification algorithm.
 */

#include <memory>
#include <type_traits>

#include "graph/Node.h"

namespace lod {
namespace operation {
/// @brief Pointer to the evaluated element.
template <typename Element>
class ElementPointer {
public:
    using type = std::add_pointer_t<Element>;

    ElementPointer() noexcept = default;
    ElementPointer(type element) noexcept : m_ptr(std::move(element)) {}

    /// @brief Access the evaluated element.
    type get() const noexcept;
    /// @brief Determine if the stored pointer is valid.
    bool valid() const noexcept;

private:
    type m_ptr = nullptr;
};
/// @brief Specialization for shared_ptr as element.
template <typename Element>
class ElementPointer<std::shared_ptr<Element>> {
public:
    using type = std::weak_ptr<Element>;

    ElementPointer() noexcept = default;
    ElementPointer(type element) noexcept : m_ptr(std::move(element)) {}
    ElementPointer(const std::shared_ptr<Element> &element) noexcept;

    /// @brief Access the evaluated element.
    std::shared_ptr<Element> get() const noexcept;
    /// @brief Determine if the stored pointer is valid.
    bool valid() const noexcept;

private:
    type m_ptr;
};

/// @brief Simple operation cost measurement.
template <typename Element, typename ErrorType = float>
class Simple {
public:
    using cost_type = ErrorType;

    explicit Simple(ElementPointer<Element> element, ErrorType cost) noexcept;

    /// @brief Access stored element.
    const ElementPointer<Element> &element() const noexcept;
    /// @brief Access stored cost.
    const ErrorType &cost() const noexcept;

    /// @brief Check the poiter validity.
    explicit operator bool() const noexcept;
    /// @brief Compare operations.
    bool operator<(const Simple &other) const noexcept;
    bool operator==(const Simple &other) const noexcept;
    bool operator!=(const Simple &other) const noexcept;
    /// @brief Compare cost of operation.
    bool operator<(const ErrorType &cost) const noexcept;

private:
    ElementPointer<Element> m_element;
    cost_type               m_cost = static_cast<ErrorType>(0);
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
typename ElementPointer<Element>::type ElementPointer<Element>::get() const
    noexcept
{
    return m_ptr;
}

template <typename Element>
bool ElementPointer<Element>::valid() const noexcept
{
    return m_ptr != nullptr;
}

template <typename Element>
ElementPointer<std::shared_ptr<Element>>::ElementPointer(
    const std::shared_ptr<Element> &element) noexcept
    : m_ptr(element)
{
}

template <typename Element>
std::shared_ptr<Element> ElementPointer<std::shared_ptr<Element>>::get() const
    noexcept
{
    return m_ptr.lock();
}

template <typename Element>
bool ElementPointer<std::shared_ptr<Element>>::valid() const noexcept
{
    return !m_ptr.expired();
}

template <typename Element, typename ErrorType>
Simple<Element, ErrorType>::Simple(
    ElementPointer<Element> element, ErrorType cost) noexcept
    : m_element(std::move(element)), m_cost(std::move(cost))
{
}

template <typename Element, typename ErrorType>
const ElementPointer<Element> &Simple<Element, ErrorType>::element() const
    noexcept
{
    return m_element;
}

template <typename Element, typename ErrorType>
const ErrorType &Simple<Element, ErrorType>::cost() const noexcept
{
    return m_cost;
}

template <typename Element, typename ErrorType>
Simple<Element, ErrorType>::operator bool() const noexcept
{
    return m_element.valid();
}

template <typename Element, typename ErrorType>
bool Simple<Element, ErrorType>::operator<(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return m_cost < other.m_cost;
}

template <typename Element, typename ErrorType>
bool Simple<Element, ErrorType>::operator==(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return m_element.get() == other.m_element.get() && m_cost == other.m_cost;
}

template <typename Element, typename ErrorType>
bool Simple<Element, ErrorType>::operator!=(
    const Simple<Element, ErrorType> &other) const noexcept
{
    return !operator==(other);
}

template <typename Element, typename ErrorType>
bool Simple<Element, ErrorType>::operator<(const ErrorType &cost) const noexcept
{
    return m_cost < cost;
}

template <typename Element, typename ErrorType>
VertexPlacement<Element, ErrorType>::VertexPlacement(
    ElementPointer<Element> element,
    ErrorType               cost,
    position_type           position) noexcept
    : Simple<Element, ErrorType>(std::move(element), std::move(cost)),
      m_hint(std::move(position))
{
}
}  // namespace operation
}  // namespace lod

#endif /* end of include guard: PROTOCOL_H_DJOVTO94 */
