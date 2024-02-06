// Copyright (c) 2024 Bronek Kozicki
//
// Distributed under the ISC License. See accompanying file LICENSE.md
// or copy at https://opensource.org/licenses/ISC

#include "functional/optional.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("optional", "[optional][pack]")
{
  constexpr fn::optional<int> a1{1};
  constexpr fn::optional<int> a2{2};
  constexpr auto a1_a2 = a1 & a2;
  static_assert(a1_a2.has_value()
                && a1_a2.value().invoke([](int i, int j) constexpr -> bool { return i == 1 && j == 2; }));
  static_assert((a1_a2 & fn::optional<bool>(false)).value().invoke([](int i, int j, bool k) -> bool {
    return i == 1 && j == 2 && k == false;
  }));
  static_assert(not(a1_a2 & fn::optional<bool>()).has_value());

  constexpr auto a1_d05 = a1 & fn::optional<double>(0.5);
  static_assert(a1_d05.has_value()
                && a1_d05.value().invoke([](int i, double j) constexpr -> bool { return i == 1 && j == 0.5; }));

  static_assert(not(a1 & fn::optional<bool>{}).has_value());
  static_assert(not(fn::optional<bool>{} & a2).has_value());
  static_assert(not(fn::optional<bool>{} & fn::optional<int>{}).has_value());
}
