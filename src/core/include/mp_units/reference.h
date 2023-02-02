// The MIT License (MIT)
//
// Copyright (c) 2018 Mateusz Pusz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <mp_units/bits/quantity_concepts.h>
#include <mp_units/bits/reference_concepts.h>
#include <mp_units/bits/representation_concepts.h>
#include <mp_units/quantity_spec.h>

namespace mp_units {

namespace detail {

template<AssociatedUnit U>
[[nodiscard]] consteval auto get_associated_quantity(U);

template<typename U, auto... Vs>
[[nodiscard]] consteval auto get_associated_quantity(power<U, Vs...>)
{
  return get_associated_quantity(U{});
}

template<typename... Us>
[[nodiscard]] consteval auto get_associated_quantity(type_list<Us...>)
{
  return (dimensionless * ... * get_associated_quantity(Us{}));
}

template<AssociatedUnit U>
[[nodiscard]] consteval auto get_associated_quantity(U)
{
  if constexpr (requires { U::reference_unit; })
    return get_associated_quantity(U::reference_unit);
  else if constexpr (requires { typename U::_num_; })
    return get_associated_quantity(typename U::_num_{}) / get_associated_quantity(typename U::_den_{});
  else if constexpr (requires { U::base_quantity; })
    return U::base_quantity;
}

}  // namespace detail

[[nodiscard]] consteval QuantitySpec auto get_quantity_spec(AssociatedUnit auto u)
{
  return detail::get_associated_quantity(u);
}

template<auto Q, auto U>
[[nodiscard]] consteval QuantitySpec auto get_quantity_spec(reference<Q, U>)
{
  return Q;
}

[[nodiscard]] consteval Unit auto get_unit(AssociatedUnit auto u) { return u; }

template<auto Q, auto U>
[[nodiscard]] consteval Unit auto get_unit(reference<Q, U>)
{
  return U;
}


template<Reference auto R, RepresentationOf<get_quantity_spec(R).character> Rep>
class quantity;

/**
 * @brief Quantity reference type
 *
 * Quantity reference describes all the properties of a quantity besides its
 * representation type.
 *
 * In most cases this class template is not explicitly instantiated by the user.
 * It is implicitly instantiated by the library's framework while binding a quantity
 * specification with a compatible unit.
 *
 * @code{.cpp}
 * Reference auto kmph = isq::speed[km / h];
 * QuantityOf<isq::speed[km / h]> auto speed = 90 * kmph;
 * @endcode
 *
 * The following syntaxes are not allowed:
 * `2 / kmph`, `kmph * 3`, `kmph / 4`, `70 * isq::length[km] / isq:time[h]`.
 */
template<QuantitySpec auto Q, Unit auto U>
struct reference {
  template<RepresentationOf<Q.character> Rep>
  // TODO can we somehow return an explicit quantity type here?
  [[nodiscard]] constexpr std::same_as<quantity<reference{}, Rep>> auto operator()(Rep&& value) const
  {
    return quantity<reference{}, Rep>(std::forward<Rep>(value));
  }
};

template<auto Q1, auto U1, auto Q2, auto U2>
[[nodiscard]] consteval bool operator==(reference<Q1, U1>, reference<Q2, U2>)
{
  return Q1 == Q2 && U1 == U2;
}

template<auto Q1, auto U1, AssociatedUnit U2>
[[nodiscard]] consteval bool operator==(reference<Q1, U1>, U2 u2)
{
  return Q1 == get_quantity_spec(u2) && U1 == u2;
}

template<auto Q1, auto U1, auto Q2, auto U2>
[[nodiscard]] consteval reference<Q1 * Q2, U1 * U2> operator*(reference<Q1, U1>, reference<Q2, U2>)
{
  return {};
}

template<auto Q1, auto U1, AssociatedUnit U2>
[[nodiscard]] consteval reference<Q1 * get_quantity_spec(U2{}), U1* U2{}> operator*(reference<Q1, U1>, U2)
{
  return {};
}

template<AssociatedUnit U1, auto Q2, auto U2>
[[nodiscard]] consteval reference<get_quantity_spec(U1{}) * Q2, U1{} * U2> operator*(U1, reference<Q2, U2>)
{
  return {};
}

template<auto Q1, auto U1, auto Q2, auto U2>
[[nodiscard]] consteval reference<Q1 / Q2, U1 / U2> operator/(reference<Q1, U1>, reference<Q2, U2>)
{
  return {};
}

template<auto Q1, auto U1, AssociatedUnit U2>
[[nodiscard]] consteval reference<Q1 / get_quantity_spec(U2{}), U1 / U2{}> operator/(reference<Q1, U1>, U2)
{
  return {};
}

template<AssociatedUnit U1, auto Q2, auto U2>
[[nodiscard]] consteval reference<get_quantity_spec(U1{}) / Q2, U1{} / U2> operator/(U1, reference<Q2, U2>)
{
  return {};
}

template<Reference R, RepresentationOf<get_quantity_spec(R{}).character> Rep>
[[nodiscard]] constexpr quantity<R{}, Rep> operator*(const Rep& lhs, R)
{
  return quantity<R{}, Rep>(lhs);
}

void /*Use `q * (1 * r)` rather than `q * r`.*/ operator*(Quantity auto, Reference auto) = delete;

template<auto Q1, auto U1, auto Q2, auto U2>
[[nodiscard]] consteval bool interconvertible(reference<Q1, U1>, reference<Q2, U2>)
{
  return interconvertible(Q1, Q2) && interconvertible(U1, U2);
}

template<auto Q1, auto U1, AssociatedUnit U2>
[[nodiscard]] consteval bool interconvertible(reference<Q1, U1>, U2 u2)
{
  return interconvertible(Q1, get_quantity_spec(u2)) && interconvertible(U1, u2);
}

template<AssociatedUnit U1, auto Q2, auto U2>
[[nodiscard]] consteval bool interconvertible(U1 u1, reference<Q2, U2> r2)
{
  return interconvertible(r2, u1);
}

[[nodiscard]] consteval auto common_reference(AssociatedUnit auto u1, AssociatedUnit auto u2,
                                              AssociatedUnit auto... rest)
  requires requires {
    {
      common_unit(u1, u2, rest...)
    } -> AssociatedUnit;
  }
{
  return common_unit(u1, u2, rest...);
}

[[nodiscard]] consteval auto common_reference(Reference auto r1, Reference auto r2, Reference auto... rest)
  requires requires {
    {
      common_quantity_spec(get_quantity_spec(r1), get_quantity_spec(r2), get_quantity_spec(rest)...)
    } -> QuantitySpec;
    {
      common_unit(get_unit(r1), get_unit(r2), get_unit(rest)...)
    } -> Unit;
  }
{
  return reference<common_quantity_spec(get_quantity_spec(r1), get_quantity_spec(r2), get_quantity_spec(rest)...),
                   common_unit(get_unit(r1), get_unit(r2), get_unit(rest)...)>{};
}

}  // namespace mp_units
