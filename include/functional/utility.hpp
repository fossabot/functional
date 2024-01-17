// Copyright (c) 2024 Bronek Kozicki
//
// Distributed under the ISC License. See accompanying file LICENSE.md
// or copy at https://opensource.org/licenses/ISC

#ifndef INCLUDE_FUNCTIONAL_UTILITY
#define INCLUDE_FUNCTIONAL_UTILITY

#include "functional/detail/traits.hpp"

// This FWD macro is a functional equivalent to std::forward<decltype(v)>(v),
// but it saves compilation time (and typing) when used frequently.
#define FWD(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)

namespace fn {

template <typename T> using as_value_t = decltype(detail::_as_value<T>);

template <typename T, typename V>
using apply_const_t = decltype(detail::_apply_const<T &, V>);

// NOTE: Unlike apply_const_t above this is not exact: prvalue parameters are
// changed to xvalue. This is meant to disable copying of the return value.
template <typename T>
constexpr auto apply_const(auto &&v) noexcept -> decltype(auto)
{
  return static_cast<apply_const_t<T, decltype(v)>>(v);
}

} // namespace fn

#endif // INCLUDE_FUNCTIONAL_UTILITY
