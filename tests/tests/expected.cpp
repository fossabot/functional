// Copyright (c) 2024 Bronek Kozicki
//
// Distributed under the ISC License. See accompanying file LICENSE.md
// or copy at https://opensource.org/licenses/ISC

#include "functional/expected.hpp"
#include "functional/and_then.hpp"

#include <catch2/catch_all.hpp>

#include <cstdio>
#include <string>

namespace {
enum Error {
  Unknown,
  NoSuchFile,
  CannotConnect,
  IllformedInput,
};
} // namespace

TEST_CASE("expected", "[expected][pack]")
{
  constexpr fn::expected<int, Error> a1{1};
  constexpr fn::expected<int, Error> a2{2};
  constexpr fn::expected<bool, Error> f{std::unexpect, CannotConnect};
  constexpr auto a1_a2 = a1 & a2;
  static_assert(a1_a2.has_value()
                && a1_a2.value().invoke([](int i, int j) constexpr -> bool { return i == 1 && j == 2; }));
  static_assert((a1_a2 & fn::expected<bool, Error>(false)).value().invoke([](int i, int j, bool k) -> bool {
    return i == 1 && j == 2 && k == false;
  }));
  static_assert((a1_a2 & f).error() == CannotConnect);
  static_assert((a1 & a2 & f).error() == CannotConnect);
  static_assert((a1 & f & a2).error() == CannotConnect);
  static_assert((f & a1 & a2).error() == CannotConnect);

  constexpr auto a1_d05 = a1 & fn::expected<double, Error>(0.5);
  static_assert(a1_d05.has_value()
                && a1_d05.value().invoke([](int i, double j) constexpr -> bool { return i == 1 && j == 0.5; }));

  static_assert((f & fn::expected<bool, Error>{std::unexpect, NoSuchFile}).error() == CannotConnect);
  static_assert((fn::expected<bool, Error>{std::unexpect, NoSuchFile} & f).error() == NoSuchFile);

  constexpr auto fn = [](auto &&, auto &&) constexpr noexcept -> fn::expected<int, Error> { return 0; };
  static_assert(fn::invocable_and_then<decltype(fn), fn::expected<fn::pack<int, int>, Error>>);
  static_assert(std::invocable<fn::and_then_t::apply, fn::expected<fn::pack<int, int>, Error>, decltype(fn)>);

  static_assert((fn::expected<int, Error>(20)                                            //
                     & fn::expected<int, Error>(1)                                       //
                     & fn::expected<double, Error>(0.5)                                  //
                 | fn::and_then([](int i, int j, double k) -> fn::expected<int, Error> { //
                     return (i + j) / k;
                   }))
                    .value()
                == 42);

  static_assert((                                                                       //
                    fn::expected<int, Error>(std::unexpect, Unknown)                    //
                        & fn::expected<int, Error>(1)                                   //
                        & fn::expected<double, Error>(0.5)                              //
                    | fn::and_then([](int i, int j, double k)                           //
                                   -> fn::expected<int, Error> { return (i + j) / k; }) //
                    )
                    .error()
                == Unknown);
}
