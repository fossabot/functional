// Copyright (c) 2024 Bronek Kozicki
//
// Distributed under the ISC License. See accompanying file LICENSE.md
// or copy at https://opensource.org/licenses/ISC

#include "functional/sum.hpp"
#include "functional/utility.hpp"

#include <catch2/catch_all.hpp>

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct TestType final {
  static int count;
  TestType() noexcept { ++count; }
  ~TestType() noexcept { --count; }
};
int TestType::count = 0;

struct NonCopyable final {
  int v;

  constexpr operator int() const { return v; }
  constexpr NonCopyable(int i) noexcept : v(i) {}
  NonCopyable(NonCopyable const &) = delete;
  NonCopyable &operator=(NonCopyable const &) = delete;
};

} // anonymous namespace

TEST_CASE("sum basic functionality tests", "[sum]")
{
  // NOTE This test looks very similar to test in choice.cpp - for good reason.

  using fn::some_in_place_type;
  using fn::sum;

  WHEN("sum_for")
  {
    static_assert(std::same_as<fn::sum_for<int>, fn::sum<int>>);
    static_assert(std::same_as<fn::sum_for<int, bool>, fn::sum<bool, int>>);
    static_assert(std::same_as<fn::sum_for<bool, int>, fn::sum<bool, int>>);
    static_assert(std::same_as<fn::sum_for<int, NonCopyable>, fn::sum<NonCopyable, int>>);
    static_assert(std::same_as<fn::sum_for<NonCopyable, int>, fn::sum<NonCopyable, int>>);
    static_assert(std::same_as<fn::sum_for<int, bool, NonCopyable>, fn::sum<NonCopyable, bool, int>>);
  }

  WHEN("invocable")
  {
    using type = sum<TestType, int>;
    static_assert(fn::detail::typelist_invocable<decltype([](auto) {}), type &>);
    static_assert(fn::detail::typelist_invocable<decltype([](auto &) {}), type &>);
    static_assert(fn::detail::typelist_invocable<decltype([](auto const &) {}), type &>);
    static_assert(fn::detail::typelist_invocable<decltype(fn::overload{[](int &) {}, [](TestType &) {}}), type &>);
    static_assert(fn::detail::typelist_invocable<decltype(fn::overload{[](int) {}, [](TestType) {}}), type const &>);

    static_assert(not fn::detail::typelist_invocable<decltype([](TestType &) {}), type &>); // missing int
    static_assert(not fn::detail::typelist_invocable<decltype([](int &) {}), type &>);      // missing TestType
    static_assert(not fn::detail::typelist_invocable<decltype(fn::overload{[](int &&) {}, [](TestType &&) {}}),
                                                     type &>); // cannot bind lvalue to rvalue-reference
    static_assert(
        not fn::detail::typelist_invocable<decltype([](auto &) {}), type &&>); // cannot bind rvalue to lvalue-reference
    static_assert(not fn::detail::typelist_invocable<decltype([](auto, auto &) {}), type &>); // bad arity
    static_assert(not fn::detail::typelist_invocable<decltype(fn::overload{[](int &) {}, [](TestType &) {}}),
                                                     type const &>); // cannot bind const to non-const reference

    static_assert(fn::detail::typelist_invocable<decltype([](auto &) {}), sum<NonCopyable> &>);
    static_assert(
        not fn::detail::typelist_invocable<decltype([](auto) {}), NonCopyable &>); // copy-constructor not available
  }

  WHEN("type_invocable")
  {
    using type = sum<TestType, int>;
    static_assert(fn::detail::typelist_type_invocable<decltype([](auto, auto) {}), type &>);
    static_assert(fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto) {}), type &>);
    static_assert(fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto &) {}), type &>);
    static_assert(
        fn::detail::typelist_type_invocable<decltype(fn::overload{[](some_in_place_type auto, int &) {},
                                                                  [](some_in_place_type auto, TestType &) {}}),
                                            type &>);
    static_assert(fn::detail::typelist_type_invocable<decltype(fn::overload{[](some_in_place_type auto, int) {},
                                                                            [](some_in_place_type auto, TestType) {}}),
                                                      type const &>);
    static_assert(not fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, TestType &) {}),
                                                          type &>); // missing int
    static_assert(not fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, int &) {}),
                                                          type &>); // missing TestType
    static_assert(
        not fn::detail::typelist_type_invocable<decltype(fn::overload{[](some_in_place_type auto, int &&) {},
                                                                      [](some_in_place_type auto, TestType &&) {}}),
                                                type &>); // cannot bind lvalue to rvalue-reference
    static_assert(not fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto &) {}),
                                                          type &&>); // cannot bind rvalue to lvalue-reference
    static_assert(not fn::detail::typelist_type_invocable<decltype([](auto) {}), type &>); // bad arity
    static_assert(
        not fn::detail::typelist_type_invocable<decltype(fn::overload{[](some_in_place_type auto, int &) {},
                                                                      [](some_in_place_type auto, TestType &) {}}),
                                                type const &>); // cannot bind const to non-const reference

    static_assert(
        fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto &) {}), sum<NonCopyable> &>);
    static_assert(not fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto) {}),
                                                          NonCopyable &>); // copy-constructor not available

    static_assert(
        fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto &) {}), sum<NonCopyable> &>);
    static_assert(not fn::detail::typelist_type_invocable<decltype([](some_in_place_type auto, auto) {}),
                                                          NonCopyable &>); // copy-constructor not available
  }

  WHEN("check destructor call")
  {
    {
      sum<TestType> s{std::in_place_type<TestType>};
      static_assert(decltype(s)::has_type<TestType>);
      static_assert(not decltype(s)::has_type<int>);
      CHECK(s.has_value(std::in_place_type<TestType>));
      CHECK(s.template has_value<TestType>());
      CHECK(TestType::count == 1);
    }
    CHECK(TestType::count == 0);
  }

  WHEN("single parameter constructor")
  {
    constexpr sum<int> a = 12;
    static_assert(a == sum{12});

    constexpr sum<bool> b{false};
    static_assert(b == sum{false});

    WHEN("CTAD")
    {
      sum a{42};
      static_assert(std::is_same_v<decltype(a), sum<int>>);
      CHECK(a == sum<int>{42});

      constexpr sum b{false};
      static_assert(std::is_same_v<decltype(b), sum<bool> const>);
      static_assert(b == sum<bool>{false});

      constexpr auto c = sum{std::array<int, 3>{3, 14, 15}};
      static_assert(std::is_same_v<decltype(c), sum<std::array<int, 3>> const>);
      static_assert(
          c.invoke_to([](auto &&a) -> bool { return a.size() == 3 && a[0] == 3 && a[1] == 14 && a[2] == 15; }));
    }

    WHEN("move from rvalue")
    {
      using T = fn::sum<bool, int>;
      constexpr auto fn = [](auto i) constexpr noexcept -> T { return {std::move(i)}; };
      constexpr auto a = fn(true);
      static_assert(std::is_same_v<decltype(a), T const>);
      static_assert(a.has_value<bool>());

      constexpr auto b = fn(12);
      static_assert(std::is_same_v<decltype(b), T const>);
      static_assert(b.has_value<int>());
    }

    WHEN("copy from lvalue")
    {
      using T = fn::sum<bool, int>;
      constexpr auto fn = [](auto i) constexpr noexcept -> T { return {i}; };
      constexpr auto a = fn(true);
      static_assert(std::is_same_v<decltype(a), T const>);
      static_assert(a.has_value<bool>());

      constexpr auto b = fn(12);
      static_assert(std::is_same_v<decltype(b), T const>);
      static_assert(b.has_value<int>());
    }
  }

  WHEN("forwarding constructors (immovable)")
  {
    sum<NonCopyable> a{std::in_place_type<NonCopyable>, 42};
    CHECK(a.invoke_to([](auto &i) -> bool { return i.v == 42; }));

    WHEN("CTAD")
    {
      constexpr auto a = sum{std::in_place_type<NonCopyable>, 42};
      static_assert(std::is_same_v<decltype(a), sum<NonCopyable> const>);

      auto b = sum{std::in_place_type<NonCopyable>, 42};
      static_assert(std::is_same_v<decltype(b), sum<NonCopyable>>);
    }

    WHEN("CTAD with const")
    {
      constexpr auto a = sum{std::in_place_type<NonCopyable const>, 42};
      static_assert(std::is_same_v<decltype(a), sum<NonCopyable const> const>);

      auto b = sum{std::in_place_type<NonCopyable const>, 42};
      static_assert(std::is_same_v<decltype(b), sum<NonCopyable const>>);
    }
  }

  WHEN("forwarding constructors (aggregate)")
  {
    WHEN("regular")
    {
      sum<std::array<int, 3>> a{std::in_place_type<std::array<int, 3>>, 1, 2, 3};
      static_assert(decltype(a)::has_type<std::array<int, 3>>);
      static_assert(not decltype(a)::has_type<int>);
      CHECK(a.has_value(std::in_place_type<std::array<int, 3>>));
      CHECK(a.template has_value<std::array<int, 3>>());
      CHECK(a.invoke_to([](auto &i) -> bool {
        return std::same_as<std::array<int, 3> &, decltype(i)> && i.size() == 3 && i[0] == 1 && i[1] == 2 && i[2] == 3;
      }));
    }

    WHEN("constexpr")
    {
      constexpr sum<std::array<int, 3>> a{std::in_place_type<std::array<int, 3>>, 1, 2, 3};
      static_assert(decltype(a)::has_type<std::array<int, 3>>);
      static_assert(not decltype(a)::has_type<int>);
      static_assert(a.has_value(std::in_place_type<std::array<int, 3>>));
      static_assert(a.template has_value<std::array<int, 3>>());
      static_assert(a.invoke_to([](auto &i) -> bool {
        return std::same_as<std::array<int, 3> const &, decltype(i)> && i.size() == 3 && i[0] == 1 && i[1] == 2
               && i[2] == 3;
      }));
    }

    WHEN("CTAD")
    {
      constexpr auto a = sum{std::in_place_type<std::array<int, 3>>, 1, 2, 3};
      static_assert(std::is_same_v<decltype(a), sum<std::array<int, 3>> const>);

      auto b = sum{std::in_place_type<std::array<int, 3>>, 1, 2, 3};
      static_assert(std::is_same_v<decltype(b), sum<std::array<int, 3>>>);
    }

    WHEN("CTAD with const")
    {
      constexpr auto a = sum{std::in_place_type<std::array<int, 3> const>, 1, 2, 3};
      static_assert(std::is_same_v<decltype(a), sum<std::array<int, 3> const> const>);

      auto b = sum{std::in_place_type<std::array<int, 3> const>, 1, 2, 3};
      static_assert(std::is_same_v<decltype(b), sum<std::array<int, 3> const>>);
    }
  }

  WHEN("has_type type mismatch")
  {
    using type = sum<bool, int>;
    static_assert(type::has_type<int>);
    static_assert(type::has_type<bool>);
    static_assert(not type::has_type<double>);
    type a{std::in_place_type<int>, 42};
    CHECK(a.has_value(std::in_place_type<int>));
    CHECK(not a.has_value(std::in_place_type<bool>));
    static_assert([](auto const &a) constexpr -> bool { //
      return not requires { a.has_value(std::in_place_type<double>); };
    }(a));                                              // double is not a type member
    static_assert([](auto const &a) constexpr -> bool { //
      return not requires { a.template has_value<double>(); };
    }(a)); // double is not a type member
  }

  WHEN("equality comparison")
  {
    using type = sum<bool, int>;

    type const a{std::in_place_type<int>, 42};
    CHECK(a == type{42});
    CHECK(a != type{41});
    CHECK(a != type{false});

    CHECK(a == type{std::in_place_type<int>, 42});
    CHECK(a != type{std::in_place_type<int>, 41});
    CHECK(a != type{std::in_place_type<bool>, true});
    CHECK(not(a == type{std::in_place_type<int>, 41}));
    CHECK(not(a == type{std::in_place_type<bool>, true}));

    WHEN("constexpr")
    {
      constexpr type a{std::in_place_type<int>, 42};
      static_assert(a == 42);
      static_assert(a != 41);
      static_assert(a != false);
      static_assert(a != true);
      static_assert(not(a == 41));
      static_assert(not(a == false));
      static_assert(not(a == true));
      static_assert(a == type{std::in_place_type<int>, 42});
      static_assert(a != type{std::in_place_type<int>, 41});
      static_assert(a != type{std::in_place_type<bool>, true});
      static_assert(not(a == type{std::in_place_type<int>, 41}));
      static_assert(not(a == type{std::in_place_type<bool>, true}));

      static_assert([](auto const &a) constexpr -> bool { //
        return not requires { a == 0.5; };
      }(a));                                              // double is not a type member
      static_assert([](auto const &a) constexpr -> bool { //
        return not requires { a != 0.5; };
      }(a));                                              // double is not a type member
      static_assert([](auto const &a) constexpr -> bool { //
        return not requires { a == sum(std::in_place_type<int>, 1); };
      }(a));                                              // type mismatch sum<int>
      static_assert([](auto const &a) constexpr -> bool { //
        return not requires { a != sum(std::in_place_type<int>, 1); };
      }(a)); // type mismatch sum<int>
    }
  }

  WHEN("invoke_to")
  {
    sum<int> a{std::in_place_type<int>, 42};
    WHEN("value only")
    {
      CHECK(a.invoke_to(fn::overload([](auto) -> bool { throw 1; }, [](int &) -> bool { return true; },
                                     [](int const &) -> bool { throw 0; }, [](int &&) -> bool { throw 0; },
                                     [](int const &&) -> bool { throw 0; })));
      CHECK(std::as_const(a).invoke_to(fn::overload(
          [](auto) -> bool { throw 1; }, [](int &) -> bool { throw 0; }, [](int const &) -> bool { return true; },
          [](int &&) -> bool { throw 0; }, [](int const &&) -> bool { throw 0; })));
      CHECK(sum<int>{std::in_place_type<int>, 42}.invoke_to(fn::overload(
          [](auto) -> bool { throw 1; }, [](int &) -> bool { throw 0; }, [](int const &) -> bool { throw 0; },
          [](int &&) -> bool { return true; }, [](int const &&) -> bool { throw 0; })));
      CHECK(std::move(std::as_const(a))
                .invoke_to(fn::overload([](auto) -> bool { throw 1; }, [](int &) -> bool { throw 0; },
                                        [](int const &) -> bool { throw 0; }, [](int &&) -> bool { throw 0; },
                                        [](int const &&) -> bool { return true; })));

      WHEN("constexpr")
      {
        constexpr sum<int> a{std::in_place_type<int>, 42};
        static_assert(a.invoke_to(fn::overload(
            [](auto) -> std::false_type { return {}; }, //
            [](int &) -> std::false_type { return {}; }, [](int const &) -> std::true_type { return {}; },
            [](int &&) -> std::false_type { return {}; }, [](int const &&) -> std::false_type { return {}; })));
        static_assert(std::move(a).invoke_to(fn::overload(
            [](auto) -> std::false_type { return {}; }, //
            [](int &) -> std::false_type { return {}; }, [](int const &) -> std::false_type { return {}; },
            [](int &&) -> std::false_type { return {}; }, [](int const &&) -> std::true_type { return {}; })));
      }
    }

    WHEN("tag and value")
    {
      CHECK(a.invoke_to(fn::overload([](auto, auto) -> bool { throw 1; },
                                     [](std::in_place_type_t<int>, int &) -> bool { return true; },
                                     [](std::in_place_type_t<int>, int const &) -> bool { throw 0; },
                                     [](std::in_place_type_t<int>, int &&) -> bool { throw 0; },
                                     [](std::in_place_type_t<int>, int const &&) -> bool { throw 0; })));
      CHECK(std::as_const(a).invoke_to(fn::overload([](auto, auto) -> bool { throw 1; },
                                                    [](std::in_place_type_t<int>, int &) -> bool { throw 0; },
                                                    [](std::in_place_type_t<int>, int const &) -> bool { return true; },
                                                    [](std::in_place_type_t<int>, int &&) -> bool { throw 0; },
                                                    [](std::in_place_type_t<int>, int const &&) -> bool { throw 0; })));
      CHECK(sum<int>{std::in_place_type<int>, 42}.invoke_to(
          fn::overload([](auto, auto) -> bool { throw 1; }, [](std::in_place_type_t<int>, int &) -> bool { throw 0; },
                       [](std::in_place_type_t<int>, int const &) -> bool { throw 0; },
                       [](std::in_place_type_t<int>, int &&) -> bool { return true; },
                       [](std::in_place_type_t<int>, int const &&) -> bool { throw 0; })));
      CHECK(std::move(std::as_const(a))
                .invoke_to(fn::overload([](auto, auto) -> bool { throw 1; },
                                        [](std::in_place_type_t<int>, int &) -> bool { throw 0; },
                                        [](std::in_place_type_t<int>, int const &) -> bool { throw 0; },
                                        [](std::in_place_type_t<int>, int &&) -> bool { throw 0; },
                                        [](std::in_place_type_t<int>, int const &&) -> bool { return true; })));
      WHEN("constexpr")
      {
        constexpr sum<int> a{std::in_place_type<int>, 42};
        static_assert(
            a.invoke_to(fn::overload([](auto, auto) -> std::false_type { return {}; }, //
                                     [](std::in_place_type_t<int>, int &) -> std::false_type { return {}; },
                                     [](std::in_place_type_t<int>, int const &) -> std::true_type { return {}; },
                                     [](std::in_place_type_t<int>, int &&) -> std::false_type { return {}; },
                                     [](std::in_place_type_t<int>, int const &&) -> std::false_type { return {}; })));
        static_assert(std::move(a).invoke_to(
            fn::overload([](auto, auto) -> std::false_type { return {}; }, //
                         [](std::in_place_type_t<int>, int &) -> std::false_type { return {}; },
                         [](std::in_place_type_t<int>, int const &) -> std::false_type { return {}; },
                         [](std::in_place_type_t<int>, int &&) -> std::false_type { return {}; },
                         [](std::in_place_type_t<int>, int const &&) -> std::true_type { return {}; })));
      }
    }
  }
}

namespace {
struct PassThrough {
  auto operator()(std::equality_comparable auto &&v) const -> std::remove_cvref_t<decltype(v)>;
  auto operator()(fn::some_in_place_type auto, std::equality_comparable auto &&v) const
      -> std::remove_cvref_t<decltype(v)>;
};
} // namespace

TEST_CASE("sum type collapsing", "[sum][invoke][normalized]")
{
  using ::fn::overload;
  using ::fn::some_in_place_type;
  using ::fn::sum;
  using ::fn::detail::_collapsing_sum_tag;
  using ::fn::detail::_invoke_result;
  using ::fn::detail::_invoke_type_result;
  using ::fn::detail::_typelist_collapsing_sum;
  using ::fn::detail::_typelist_type_collapsing_sum;

  struct sum_double_int {};
  struct sum_bool {};
  struct sum_bool_int {};

  constexpr auto fn1 = overload{PassThrough{},
                                [](sum_double_int const &) -> sum<double, int> { throw 0; },
                                [](some_in_place_type auto, sum_double_int const &) -> sum<double, int> { throw 0; },
                                [](sum_bool const &) -> sum<bool> && { throw 0; },
                                [](some_in_place_type auto, sum_bool const &) -> sum<bool> && { throw 0; },
                                [](sum_bool_int const &) -> sum<bool, int> const { throw 0; },
                                [](some_in_place_type auto, sum_bool_int const &) -> sum<bool, int> const { throw 0; }};

  WHEN("one element")
  {
    using type = sum<double>;
    static_assert(std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<double>>);
    static_assert(
        std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<double>>);
  }

  WHEN("two elements")
  {
    using type = sum<double, int>;
    static_assert(
        std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<double, int>>);
    static_assert(
        std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<double, int>>);
  }

  WHEN("one sum, one element only")
  {
    using type = sum<sum_bool>;
    static_assert(std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool>>);
    static_assert(
        std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool>>);
  }

  WHEN("element and one sum with one element")
  {
    using type = sum<double, sum_bool>;
    static_assert(
        std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool, double>>);
    static_assert(std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type,
                               sum<bool, double>>);
  }

  WHEN("one sum with two elements")
  {
    using type = sum<sum_bool_int>;
    static_assert(
        std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool, int>>);
    static_assert(
        std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool, int>>);
  }

  WHEN("sum with two elements and sum with one element")
  {
    using type = sum<sum_bool_int, sum_bool>;
    static_assert(
        std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool, int>>);
    static_assert(
        std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type, sum<bool, int>>);
  }

  WHEN("two sums with two elements and two elements")
  {
    using type = sum<sum_bool_int, sum_double_int, double, int>;
    static_assert(std::same_as<typename _invoke_result<_collapsing_sum_tag, decltype(fn1), type &>::type,
                               sum<bool, double, int>>);
    static_assert(std::same_as<typename _invoke_type_result<_collapsing_sum_tag, decltype(fn1), type &>::type,
                               sum<bool, double, int>>);
  }
}

TEST_CASE("sum invoke", "[sum][invoke]")
{
  using ::fn::sum;
  constexpr auto fn1 = [](auto i) noexcept -> std::size_t { return sizeof(i); };
  constexpr auto fn2 = [](auto, auto i) noexcept -> std::size_t { return sizeof(i); };

  WHEN("size 4")
  {
    using type = sum<double, int, std::string, std::string_view>;
    static_assert(type::size == 4);

    WHEN("element v0 set")
    {
      type a{std::in_place_type<double>, 0.5};
      CHECK(a.data.v0 == 0.5);
      WHEN("value only")
      {
        static_assert(type{0.5}.invoke(fn1) == sum{8ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto) -> int { throw 1; }, [](double &i) -> bool { return i == 0.5; },
                      [](double const &) -> bool { throw 0; }, [](double &&) -> bool { throw 0; },
                      [](double const &&) -> bool { throw 0; }))
              == sum<bool, int>{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(        //
                      [](auto) -> int { throw 1; }, [](double &) -> bool { throw 0; },
                      [](double const &i) -> bool { return i == 0.5; }, [](double &&) -> bool { throw 0; },
                      [](double const &&) -> bool { throw 0; }))
              == sum<bool, int>{true});
        CHECK(type{std::in_place_type<double>, 0.5}.invoke( //
                  fn::overload(                             //
                      [](auto) -> int { throw 1; }, [](double &) -> bool { throw 0; },
                      [](double const &) -> bool { throw 0; }, [](double &&i) -> bool { return i == 0.5; },
                      [](double const &&) -> bool { throw 0; }))
              == sum<bool, int>{true});
        CHECK(std::move(std::as_const(a))
                  .invoke(          //
                      fn::overload( //
                          [](auto) -> int { throw 1; }, [](double &) -> bool { throw 0; },
                          [](double const &) -> bool { throw 0; }, [](double &&) -> bool { throw 0; },
                          [](double const &&i) -> bool { return i == 0.5; }))
              == sum<bool, int>{true});
      }
      WHEN("tag and value")
      {
        static_assert(type{0.5}.invoke(fn2) == sum{8ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto, auto) -> int { throw 1; },
                      [](std::in_place_type_t<double>, double &i) -> bool { return i == 0.5; },
                      [](std::in_place_type_t<double>, double const &) -> bool { throw 0; },
                      [](std::in_place_type_t<double>, double &&) -> bool { throw 0; },
                      [](std::in_place_type_t<double>, double const &&) -> bool { throw 0; }))
              == sum<bool, int>{true});
        CHECK(
            std::as_const(a).invoke( //
                fn::overload(        //
                    [](auto, auto) -> int { throw 1; }, [](std::in_place_type_t<double>, double &) -> bool { throw 0; },
                    [](std::in_place_type_t<double>, double const &i) -> bool { return i == 0.5; },
                    [](std::in_place_type_t<double>, double &&) -> bool { throw 0; },
                    [](std::in_place_type_t<double>, double const &&) -> bool { throw 0; }))
            == sum<bool, int>{true});
        CHECK(
            type{std::in_place_type<double>, 0.5}.invoke( //
                fn::overload(                             //
                    [](auto, auto) -> int { throw 1; }, [](std::in_place_type_t<double>, double &) -> bool { throw 0; },
                    [](std::in_place_type_t<double>, double const &) -> bool { throw 0; },
                    [](std::in_place_type_t<double>, double &&i) -> bool { return i == 0.5; },
                    [](std::in_place_type_t<double>, double const &&) -> bool { throw 0; }))
            == sum<bool, int>{true});
        CHECK(std::move(std::as_const(a))
                  .invoke(          //
                      fn::overload( //
                          [](auto, auto) -> int { throw 1; },
                          [](std::in_place_type_t<double>, double &) -> bool { throw 0; },
                          [](std::in_place_type_t<double>, double const &) -> bool { throw 0; },
                          [](std::in_place_type_t<double>, double &&) -> bool { throw 0; },
                          [](std::in_place_type_t<double>, double const &&i) -> bool { return i == 0.5; }))
              == sum<bool, int>{true});
      }
    }

    WHEN("element v1 set")
    {
      type a{std::in_place_type<int>, 42};
      CHECK(a.data.v1 == 42);

      WHEN("value only")
      {
        static_assert(type{42}.invoke(fn1) == sum{4ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto) -> bool { throw 1; }, [](int &i) -> bool { return i == 42; },
                      [](int const &) -> bool { throw 0; }, [](int &&) -> bool { throw 0; },
                      [](int const &&) -> bool { throw 0; }))
              == sum{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(        //
                      [](auto) -> bool { throw 1; }, [](int &) -> bool { throw 0; },
                      [](int const &i) -> bool { return i == 42; }, [](int &&) -> bool { throw 0; },
                      [](int const &&) -> bool { throw 0; }))
              == sum{true});
        CHECK(
            sum<int>{std::in_place_type<int>, 42}.invoke( //
                fn::overload(                             //
                    [](auto) -> bool { throw 1; }, [](int &) -> bool { throw 0; }, [](int const &) -> bool { throw 0; },
                    [](int &&i) -> bool { return i == 42; }, [](int const &&) -> bool { throw 0; }))
            == sum{true});
        CHECK(std::move(std::as_const(a))
                  .invoke(          //
                      fn::overload( //
                          [](auto) -> bool { throw 1; }, [](int &) -> bool { throw 0; },
                          [](int const &) -> bool { throw 0; }, [](int &&) -> bool { throw 0; },
                          [](int const &&i) -> bool { return i == 42; }))
              == sum{true});
      }

      WHEN("tag and value")
      {
        static_assert(type{42}.invoke(fn2) == sum{4ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto, auto) -> bool { throw 1; },
                      [](std::in_place_type_t<int>, int &i) -> bool { return i == 42; },
                      [](std::in_place_type_t<int>, int const &) -> bool { throw 0; },
                      [](std::in_place_type_t<int>, int &&) -> bool { throw 0; },
                      [](std::in_place_type_t<int>, int const &&) -> bool { throw 0; }))
              == sum{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(        //
                      [](auto, auto) -> bool { throw 1; }, [](std::in_place_type_t<int>, int &) -> bool { throw 0; },
                      [](std::in_place_type_t<int>, int const &i) -> bool { return i == 42; },
                      [](std::in_place_type_t<int>, int &&) -> bool { throw 0; },
                      [](std::in_place_type_t<int>, int const &&) -> bool { throw 0; }))
              == sum{true});
        CHECK(sum<int>{std::in_place_type<int>, 42}.invoke( //
                  fn::overload(                             //
                      [](auto, auto) -> bool { throw 1; }, [](std::in_place_type_t<int>, int &) -> bool { throw 0; },
                      [](std::in_place_type_t<int>, int const &) -> bool { throw 0; },
                      [](std::in_place_type_t<int>, int &&i) -> bool { return i == 42; },
                      [](std::in_place_type_t<int>, int const &&) -> bool { throw 0; }))
              == sum{true});
        CHECK(
            std::move(std::as_const(a))
                .invoke(          //
                    fn::overload( //
                        [](auto, auto) -> bool { throw 1; }, [](std::in_place_type_t<int>, int &) -> bool { throw 0; },
                        [](std::in_place_type_t<int>, int const &) -> bool { throw 0; },
                        [](std::in_place_type_t<int>, int &&) -> bool { throw 0; },
                        [](std::in_place_type_t<int>, int const &&i) -> bool { return i == 42; }))
            == sum{true});
      }
    }

    WHEN("element v2 set")
    {
      type a{std::in_place_type<std::string>, "bar"};
      CHECK(a.data.v2 == "bar");
      WHEN("value only")
      {
        // TODO Change single CHECK below to static_assert when supported by Clang
        CHECK(type{std::in_place_type<std::string>, "bar"}.invoke(fn1) == sum{32ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto) -> sum<bool, std::string> { throw 1; },
                      [](std::string &i) -> bool { return i == "bar"; }, [](std::string const &) -> bool { throw 0; },
                      [](std::string &&) -> bool { throw 0; }, [](std::string const &&) -> bool { throw 0; }))
              == sum<bool, std::string>{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(        //
                      [](auto) -> sum<bool, std::string> { throw 1; }, [](std::string &) -> bool { throw 0; },
                      [](std::string const &i) -> bool { return i == "bar"; }, [](std::string &&) -> bool { throw 0; },
                      [](std::string const &&) -> bool { throw 0; }))
              == sum<bool, std::string>{true});
        CHECK(type{std::in_place_type<std::string>, "bar"}.invoke( //
                  fn::overload(                                    //
                      [](auto) -> sum<bool, std::string> { throw 1; }, [](std::string &) -> bool { throw 0; },
                      [](std::string const &) -> bool { throw 0; }, [](std::string &&i) -> bool { return i == "bar"; },
                      [](std::string const &&) -> bool { throw 0; }))
              == sum<bool, std::string>{true});
        CHECK(std::move(std::as_const(a))
                  .invoke(          //
                      fn::overload( //
                          [](auto) -> sum<bool, std::string> { throw 1; }, [](std::string &) -> bool { throw 0; },
                          [](std::string const &) -> bool { throw 0; }, [](std::string &&) -> bool { throw 0; },
                          [](std::string const &&i) -> bool { return i == "bar"; }))
              == sum<bool, std::string>{true});
      }
      WHEN("tag and value")
      {
        // TODO Change single CHECK below to static_assert when supported by Clang
        CHECK(type{std::in_place_type<std::string>, "bar"}.invoke(fn2) == sum{32ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto, auto) -> sum<bool, std::string> { throw 1; },
                      [](std::in_place_type_t<std::string>, std::string &i) -> bool { return i == "bar"; },
                      [](std::in_place_type_t<std::string>, std::string const &) -> bool { throw 0; },
                      [](std::in_place_type_t<std::string>, std::string &&) -> bool { throw 0; },
                      [](std::in_place_type_t<std::string>, std::string const &&) -> bool { throw 0; }))
              == sum<bool, std::string>{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(        //
                      [](auto, auto) -> sum<bool, std::string> { throw 1; },
                      [](std::in_place_type_t<std::string>, std::string &) -> bool { throw 0; },
                      [](std::in_place_type_t<std::string>, std::string const &i) -> bool { return i == "bar"; },
                      [](std::in_place_type_t<std::string>, std::string &&) -> bool { throw 0; },
                      [](std::in_place_type_t<std::string>, std::string const &&) -> bool { throw 0; }))
              == sum<bool, std::string>{true});
        CHECK(type{std::in_place_type<std::string>, "bar"}.invoke( //
                  fn::overload(                                    //
                      [](auto, auto) -> sum<bool, std::string> { throw 1; },
                      [](std::in_place_type_t<std::string>, std::string &) -> bool { throw 0; },
                      [](std::in_place_type_t<std::string>, std::string const &) -> bool { throw 0; },
                      [](std::in_place_type_t<std::string>, std::string &&i) -> bool { return i == "bar"; },
                      [](std::in_place_type_t<std::string>, std::string const &&) -> bool { throw 0; }))
              == sum<bool, std::string>{true});
        CHECK(std::move(std::as_const(a))
                  .invoke(          //
                      fn::overload( //
                          [](auto, auto) -> sum<bool, std::string> { throw 1; },
                          [](std::in_place_type_t<std::string>, std::string &) -> bool { throw 0; },
                          [](std::in_place_type_t<std::string>, std::string const &) -> bool { throw 0; },
                          [](std::in_place_type_t<std::string>, std::string &&) -> bool { throw 0; },
                          [](std::in_place_type_t<std::string>, std::string const &&i) -> bool { return i == "bar"; }))
              == sum<bool, std::string>{true});
      }
    }

    WHEN("element v3 set")
    {
      type a{std::in_place_type<std::string_view>, "baz"};
      CHECK(a.data.v3 == "baz");
      WHEN("value only")
      {
        static_assert(type{std::in_place_type<std::string_view>, "baz"}.invoke(fn1) == sum{16ul});
        CHECK(a.invoke(         //
                  fn::overload( //
                      [](auto) -> sum<int, std::string_view> { throw 1; },
                      [](std::string_view &i) -> sum<bool, int> { return {i == "baz"}; },
                      [](std::string_view const &) -> sum<bool, int> { throw 0; },
                      [](std::string_view &&) -> sum<bool, int> { throw 0; },
                      [](std::string_view const &&) -> sum<bool, int> { throw 0; }))
              == sum<bool, int, std::string_view>{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(        //
                      [](auto) -> sum<int, std::string_view> { throw 1; },
                      [](std::string_view &) -> sum<bool, int> { throw 0; },
                      [](std::string_view const &i) -> sum<bool, int> { return {i == "baz"}; },
                      [](std::string_view &&) -> sum<bool, int> { throw 0; },
                      [](std::string_view const &&) -> sum<bool, int> { throw 0; }))
              == sum<bool, int, std::string_view>{true});
        CHECK(type{std::in_place_type<std::string_view>, "baz"}.invoke( //
                  fn::overload(                                         //
                      [](auto) -> sum<int, std::string_view> { throw 1; },
                      [](std::string_view &) -> sum<bool, int> { throw 0; },
                      [](std::string_view const &) -> sum<bool, int> { throw 0; },
                      [](std::string_view &&i) -> sum<bool, int> { return {i == "baz"}; },
                      [](std::string_view const &&) -> sum<bool, int> { throw 0; }))
              == sum<bool, int, std::string_view>{true});
        CHECK(std::move(std::as_const(a))
                  .invoke(          //
                      fn::overload( //
                          [](auto) -> sum<int, std::string_view> { throw 1; },
                          [](std::string_view &) -> sum<bool, int> { throw 0; },
                          [](std::string_view const &) -> sum<bool, int> { throw 0; },
                          [](std::string_view &&) -> sum<bool, int> { throw 0; },
                          [](std::string_view const &&i) -> sum<bool, int> { return {i == "baz"}; }))
              == sum<bool, int, std::string_view>{true});
      }
      WHEN("tag and value")
      {
        static_assert(type{std::in_place_type<std::string_view>, "baz"}.invoke(fn2) == sum{16ul});
        CHECK(
            a.invoke(         //
                fn::overload( //
                    [](auto, auto) -> sum<int, std::string_view> { throw 1; },
                    [](std::in_place_type_t<std::string_view>, std::string_view &i) -> sum<bool, int> {
                      return {i == "baz"};
                    },
                    [](std::in_place_type_t<std::string_view>, std::string_view const &) -> sum<bool, int> { throw 0; },
                    [](std::in_place_type_t<std::string_view>, std::string_view &&) -> sum<bool, int> { throw 0; },
                    [](std::in_place_type_t<std::string_view>, std::string_view const &&) -> sum<bool, int> {
                      throw 0;
                    }))
            == sum<bool, int, std::string_view>{true});
        CHECK(std::as_const(a).invoke( //
                  fn::overload(
                      [](auto, auto) -> sum<int, std::string_view> { throw 1; },
                      [](std::in_place_type_t<std::string_view>, std::string_view &) -> sum<bool, int> { throw 0; },
                      [](std::in_place_type_t<std::string_view>, std::string_view const &i) -> sum<bool, int> {
                        return {i == "baz"};
                      },
                      [](std::in_place_type_t<std::string_view>, std::string_view &&) -> sum<bool, int> { throw 0; },
                      [](std::in_place_type_t<std::string_view>, std::string_view const &&) -> sum<bool, int> {
                        throw 0;
                      }))
              == sum<bool, int, std::string_view>{true});
        CHECK(
            type{std::in_place_type<std::string_view>, "baz"}.invoke( //
                fn::overload(
                    [](auto, auto) -> sum<int, std::string_view> { throw 1; },
                    [](std::in_place_type_t<std::string_view>, std::string_view &) -> sum<bool, int> { throw 0; },
                    [](std::in_place_type_t<std::string_view>, std::string_view const &) -> sum<bool, int> { throw 0; },
                    [](std::in_place_type_t<std::string_view>, std::string_view &&i) -> sum<bool, int> {
                      return {i == "baz"};
                    },
                    [](std::in_place_type_t<std::string_view>, std::string_view const &&) -> sum<bool, int> {
                      throw 0;
                    }))
            == sum<bool, int, std::string_view>{true});
        CHECK(
            std::move(std::as_const(a))
                .invoke( //
                    fn::overload(
                        [](auto, auto) -> sum<int, std::string_view> { throw 1; },
                        [](std::in_place_type_t<std::string_view>, std::string_view &) -> sum<bool, int> { throw 0; },
                        [](std::in_place_type_t<std::string_view>, std::string_view const &) -> sum<bool, int> {
                          throw 0;
                        },
                        [](std::in_place_type_t<std::string_view>, std::string_view &&) -> sum<bool, int> { throw 0; },
                        [](std::in_place_type_t<std::string_view>, std::string_view const &&i) -> sum<bool, int> {
                          return {i == "baz"};
                        }))
            == sum<bool, int, std::string_view>{true});
      }
    }
  }
}
