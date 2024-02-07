// Copyright (c) 2024 Bronek Kozicki
//
// Distributed under the ISC License. See accompanying file LICENSE.md
// or copy at https://opensource.org/licenses/ISC

#include "functional/fail.hpp"

#include <catch2/catch_all.hpp>

#include <string>
#include <utility>

namespace {
struct Error {
  std::string what;
};

struct Value final {
  int value;
  static int count;

  auto fn() const & -> Error { return {"Was " + std::to_string(value)}; }
  auto finalize() const & -> void { count += value; };
};

int Value::count = 0;

struct Derived : Error {};

} // namespace

TEST_CASE("fail", "[fail][expected][expected_value]")
{
  using namespace fn;

  using operand_t = fn::expected<int, Error>;
  constexpr auto fnValue = [](int i) -> Error { return {"Got " + std::to_string(i)}; };

  static_assert(monadic_invocable<fail_t, operand_t, decltype(fnValue)>);
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> Error { throw 0; })); // allow generic call
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](unsigned) -> Error { throw 0; })); // allow conversion
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> std::string { throw 0; })); // no conversion found
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> void { throw 0; })); // wrong return type
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> Derived { throw 0; })); // conversion
  static_assert(
      std::is_same_v<operand_t, decltype(std::declval<operand_t>() | fail([](auto...) -> Derived { throw 0; }))>);
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int &&) -> Error { throw 0; })); // alow move from rvalue
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t const, decltype(fn)>;
  }([](int &&) -> Error { throw 0; })); // disallow removing const
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t &, decltype(fn)>;
  }([](int &&) -> Error { throw 0; })); // disallow move from lvalue
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t &, decltype(fn)>;
  }([](int &) -> Error { throw 0; })); // allow lvalue binding
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t const &, decltype(fn)>;
  }([](int &) -> Error { throw 0; })); // disallow removing const
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int &) -> Error { throw 0; })); // disallow lvalue binding to rvalue

  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](std::string) -> Error { throw 0; })); // wrong type
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([]() -> Error { throw 0; })); // wrong arity
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int, int) -> Error { throw 0; })); // wrong arity

  constexpr auto fnDerived = [](auto...) -> Derived { return {}; };
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }(fnDerived)); // allow return type conversion
  static_assert(std::is_same_v<operand_t, decltype(std::declval<operand_t>() | fail(fnDerived))>);

  constexpr auto wrong = [](int) -> Error { throw 0; };

  WHEN("operand is lvalue")
  {
    WHEN("operand is value")
    {
      operand_t a{std::in_place, 12};
      using T = decltype(a | fail(fnValue));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((a | fail(fnValue)).error().what == "Got 12");
    }
    WHEN("operand is error")
    {
      operand_t a{std::unexpect, "Not good"};
      using T = decltype(a | fail(wrong));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((a //
               | fail(wrong))
                  .error()
                  .what
              == "Not good");
    }
    WHEN("calling member function")
    {
      using operand_t = fn::expected<Value, Error>;
      operand_t a{std::in_place, 12};
      using T = decltype(a | fail(&Value::fn));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((a | fail(&Value::fn)).error().what == "Was 12");
    }
  }

  WHEN("operand is rvalue")
  {
    WHEN("operand is value")
    {
      using T = decltype(operand_t{std::in_place, 12} | fail(fnValue));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((operand_t{std::in_place, 12} | fail(fnValue)).error().what == "Got 12");
    }
    WHEN("operand is error")
    {
      using T = decltype(operand_t{std::unexpect, "Not good"} | fail(wrong));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((operand_t{std::unexpect, "Not good"} //
               | fail(wrong))
                  .error()
                  .what
              == "Not good");
    }
    WHEN("calling member function")
    {
      using operand_t = fn::expected<Value, Error>;
      using T = decltype(operand_t{std::in_place, 12} | fail(&Value::fn));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((operand_t{std::in_place, 12} | fail(&Value::fn)).error().what == "Was 12");
    }
  }
}

TEST_CASE("fail", "[fail][expected][expected_void]")
{
  using namespace fn;

  using operand_t = fn::expected<void, Error>;
  int count = 0;
  auto fnValue = [&count]() -> Error { return {"Got " + std::to_string(++count)}; };

  static_assert(monadic_invocable<fail_t, operand_t, decltype(fnValue)>);
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> Error { throw 0; })); // allow generic call
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> std::string { throw 0; })); // no conversion found
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> void { throw 0; })); // wrong return type

  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto) -> Error { throw 0; })); // wrong arity
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int, int) -> Error { throw 0; })); // wrong arity

  constexpr auto fnDerived = [](auto...) -> Derived { return {}; };
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }(fnDerived)); // allow return type conversion
  static_assert(std::is_same_v<operand_t, decltype(std::declval<operand_t>() | fail(fnDerived))>);

  constexpr auto wrong = []() -> Error { throw 0; };

  WHEN("operand is lvalue")
  {
    WHEN("operand is value")
    {
      operand_t a{std::in_place};
      using T = decltype(a | fail(fnValue));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((a | fail(fnValue)).error().what == "Got 1");
    }
    WHEN("operand is error")
    {
      operand_t a{std::unexpect, "Not good"};
      using T = decltype(a | fail(wrong));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((a //
               | fail(wrong))
                  .error()
                  .what
              == "Not good");
    }
  }

  WHEN("operand is rvalue")
  {
    WHEN("operand is value")
    {
      using T = decltype(operand_t{std::in_place} | fail(fnValue));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((operand_t{std::in_place} | fail(fnValue)).error().what == "Got 1");
    }
    WHEN("operand is error")
    {
      using T = decltype(operand_t{std::unexpect, "Not good"} | fail(wrong));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE((operand_t{std::unexpect, "Not good"} //
               | fail(wrong))
                  .error()
                  .what
              == "Not good");
    }
  }
}

TEST_CASE("fail", "[fail][optional]")
{
  using namespace fn;

  using operand_t = fn::optional<int>;
  auto count = 0;
  auto fnValue = [&count](auto) { count += 1; };

  static_assert(monadic_invocable<fail_t, operand_t, decltype(fnValue)>);
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) {})); // allow generic call
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](unsigned) {})); // allow conversion
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](auto...) -> Error { throw 0; })); // wrong return type
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int &&) -> void { throw 0; })); // alow move from rvalue
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t const, decltype(fn)>;
  }([](int &&) -> void { throw 0; })); // disallow removing const
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t &, decltype(fn)>;
  }([](int &&) -> void { throw 0; })); // disallow move from lvalue
  static_assert([](auto &&fn) constexpr -> bool {
    return monadic_invocable<fail_t, operand_t &, decltype(fn)>;
  }([](int &) -> void { throw 0; })); // allow lvalue binding
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t const &, decltype(fn)>;
  }([](int &) -> void { throw 0; })); // disallow removing const
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int &) -> void { throw 0; })); // disallow lvalue binding to rvalue

  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](std::string) {})); // wrong type
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([]() {})); // wrong arity
  static_assert(not [](auto &&fn) constexpr->bool {
    return monadic_invocable<fail_t, operand_t, decltype(fn)>;
  }([](int, int) {})); // wrong arity

  constexpr auto wrong = [](auto...) { throw 0; };

  WHEN("operand is lvalue")
  {
    WHEN("operand is value")
    {
      operand_t a{12};
      using T = decltype(a | fail(fnValue));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE(not(a | fail(fnValue)).has_value());
      CHECK(count == 1);
    }
    WHEN("operand is error")
    {
      operand_t a{std::nullopt};
      using T = decltype(a | fail(wrong));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE(not(a | fail(wrong)).has_value());
      CHECK(count == 0);
    }
    WHEN("calling member function")
    {
      using operand_t = fn::optional<Value>;
      operand_t a{std::in_place, 12};
      using T = decltype(a | fail(&Value::finalize));
      static_assert(std::is_same_v<T, operand_t>);
      auto const before = Value::count;
      REQUIRE(not(a | fail(&Value::finalize)).has_value());
      CHECK(Value::count == before + 12);
    }
  }

  WHEN("operand is rvalue")
  {
    WHEN("operand is value")
    {
      using T = decltype(operand_t{std::in_place, 12} | fail(fnValue));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE(not(operand_t{std::in_place, 12} | fail(fnValue)).has_value());
      CHECK(count == 1);
    }
    WHEN("operand is error")
    {
      using T = decltype(operand_t{std::nullopt} | fail(wrong));
      static_assert(std::is_same_v<T, operand_t>);
      REQUIRE(not(operand_t{std::nullopt} //
                  | fail(wrong))
                     .has_value());
      CHECK(count == 0);
    }
    WHEN("calling member function")
    {
      using operand_t = fn::optional<Value>;
      using T = decltype(operand_t{std::in_place, 12} | fail(&Value::finalize));
      static_assert(std::is_same_v<T, operand_t>);
      auto const before = Value::count;
      REQUIRE(not(operand_t{std::in_place, 12} | fail(&Value::finalize)).has_value());
      CHECK(Value::count == before + 12);
    }
  }
}

TEST_CASE("constexpr fail expected", "[fail][constexpr][expected]")
{
  enum class Error { ThresholdExceeded, SomethingElse };
  using T = fn::expected<int, Error>;
  constexpr auto fn = [](int i) constexpr noexcept -> Error {
    if (i < 3)
      return Error::SomethingElse;
    return Error::ThresholdExceeded;
  };
  constexpr auto r1 = T{0} | fn::fail(fn);
  static_assert(r1.error() == Error::SomethingElse);
  constexpr auto r2 = T{3} | fn::fail(fn);
  static_assert(r2.error() == Error::ThresholdExceeded);

  SUCCEED();
}

TEST_CASE("constexpr fail optional", "[fail][constexpr][optional]")
{
  using T = fn::optional<int>;
  constexpr auto fn = [](int) constexpr noexcept -> void {};
  constexpr auto r1 = T{0} | fn::fail(fn);
  static_assert(not r1.has_value());

  SUCCEED();
}

namespace fn {
namespace {
struct Error {};
struct Xerror final : Error {};
struct Value final {};

template <typename E> constexpr auto fn_int = [](int) -> E { throw 0; };

template <typename E> constexpr auto fn_generic = [](auto &&...) -> E { throw 0; };

template <typename E> constexpr auto fn_int_lvalue = [](int &) -> E { throw 0; };

template <typename E> constexpr auto fn_int_rvalue = [](int &&) -> E { throw 0; };
} // namespace

// clang-format off
static_assert(invocable_fail<decltype(fn_int<Error>), fn::expected<int, Error>>);
static_assert(invocable_fail<decltype(fn_generic<Error>), fn::expected<void, Error>>);
static_assert(invocable_fail<decltype(fn_int<Xerror>), fn::expected<int, Error>>); // return type conversion
static_assert(not invocable_fail<decltype(fn_int<Error>), fn::expected<int, Xerror>>); // cannot convert error
static_assert(not invocable_fail<decltype(fn_generic<Error>), fn::expected<void, Xerror>>); // cannot convert error
static_assert(not invocable_fail<decltype(fn_generic<Error>), fn::expected<Value, Xerror>>); // cannot convert error
static_assert(not invocable_fail<decltype(fn_int<Error>), fn::expected<Value, Error>>); // wrong parameter type
static_assert(invocable_fail<decltype(fn_generic<Error>), fn::expected<Value, Error>>);

static_assert(invocable_fail<decltype(fn_int<void>), fn::optional<int>>);
static_assert(not invocable_fail<decltype(fn_int<void>), fn::optional<Value>>); // wrong parameter type
static_assert(invocable_fail<decltype(fn_generic<void>), fn::optional<Value>>);
static_assert(not invocable_fail<decltype(fn_generic<Error>), fn::optional<Value>>); // bad return type

static_assert(not invocable_fail<decltype(fn_int_lvalue<Error>), fn::expected<int, Error>>); // cannot bind temporary to lvalue
static_assert(invocable_fail<decltype(fn_int_lvalue<Error>), fn::expected<int, Error>&>);
static_assert(invocable_fail<decltype(fn_int_rvalue<Error>), fn::expected<int, Error>>);
static_assert(not invocable_fail<decltype(fn_int_rvalue<Error>), fn::expected<int, Error>&>); // cannot bind lvalue to rvalue-ref
// clang-format on
} // namespace fn
