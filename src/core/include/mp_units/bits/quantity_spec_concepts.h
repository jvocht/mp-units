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

#include <mp_units/bits/dimension_concepts.h>
#include <mp_units/bits/expression_template.h>

namespace mp_units {

#ifdef __cpp_explicit_this_parameter
template<auto...>
#else
template<typename, auto...>
#endif
struct quantity_spec;

namespace detail {

#ifdef __cpp_explicit_this_parameter
template<auto... Args>
void to_base_specialization_of_quantity_spec(const volatile quantity_spec<Args...>*);
#else
template<typename T, auto... Args>
void to_base_specialization_of_quantity_spec(const volatile quantity_spec<T, Args...>*);
#endif

#ifdef __cpp_explicit_this_parameter
template<BaseDimension auto Dim, auto... Args>
template<auto... Args>
void to_base_specialization_of_base_quantity_spec(const volatile quantity_spec<Dim, Args...>*);
#else
template<typename Self, BaseDimension auto Dim, auto... Args>
void to_base_specialization_of_base_quantity_spec(const volatile quantity_spec<Self, Dim, Args...>*);
#endif

template<typename T>
inline constexpr bool is_specialization_of_quantity_spec = false;

#ifdef __cpp_explicit_this_parameter
template<auto... Args>
inline constexpr bool is_specialization_of_quantity_spec<quantity_spec<Args...>> = true;
#else
template<typename T, auto... Args>
inline constexpr bool is_specialization_of_quantity_spec<quantity_spec<T, Args...>> = true;
#endif

/**
 * @brief Concept matching all named quantity specification types
 *
 * Satisfied by all types that derive from `quantity_spec`.
 */
template<typename T>
concept NamedQuantitySpec = requires(T* t) { detail::to_base_specialization_of_quantity_spec(t); } &&
                            (!detail::is_specialization_of_quantity_spec<T>);

}  // namespace detail

/**
 * @brief Concept matching all named base quantity specification types
 *
 * Satisfied by all types that derive from `quantity_spec` taking a base dimension
 * as a template parameter.
 */
template<typename T>
concept BaseQuantitySpec =
  detail::NamedQuantitySpec<T> && requires(T* t) { detail::to_base_specialization_of_base_quantity_spec(t); };

namespace detail {

template<typename T>
inline constexpr bool is_dimensionless = false;

template<typename T>
inline constexpr bool is_power_of_quantity_spec = requires {
  requires is_specialization_of_power<T> &&
             (NamedQuantitySpec<typename T::factor> || is_dimensionless<typename T::factor>);
};

template<typename T>
inline constexpr bool is_per_of_quantity_specs = false;

template<typename... Ts>
inline constexpr bool is_per_of_quantity_specs<per<Ts...>> =
  (... && (NamedQuantitySpec<Ts> || is_dimensionless<Ts> || is_power_of_quantity_spec<Ts>));

}  // namespace detail

template<typename T>
concept DerivedQuantitySpecExpr = detail::NamedQuantitySpec<T> || detail::is_dimensionless<T> ||
                                  detail::is_power_of_quantity_spec<T> || detail::is_per_of_quantity_specs<T>;


template<DerivedQuantitySpecExpr... Expr>
struct derived_quantity_spec;

namespace detail {

template<typename T>
inline constexpr bool is_derived_from_specialization_of_derived_quantity_spec = false;

template<typename... Args>
inline constexpr bool is_derived_from_specialization_of_derived_quantity_spec<derived_quantity_spec<Args...>> = true;

/**
 * @brief Concept matching all derived quantity specification types
 *
 * Satisfied by all `derived_quantity_spec` specializations.
 */
template<typename T>
concept DerivedQuantitySpec = detail::is_derived_from_specialization_of_derived_quantity_spec<T>;

}  // namespace detail

template<typename T>
concept QuantitySpec = detail::NamedQuantitySpec<T> || detail::DerivedQuantitySpec<T>;

}  // namespace mp_units
