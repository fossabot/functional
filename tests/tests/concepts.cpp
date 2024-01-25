// Copyright (c) 2024 Bronek Kozicki
//
// Distributed under the ISC License. See accompanying file LICENSE.md
// or copy at https://opensource.org/licenses/ISC

#include "functional/concepts.hpp"

#include <catch2/catch_all.hpp>

#include <expected>
#include <optional>

namespace fn {
static_assert(some_expected<std::expected<int, bool>>);
static_assert(some_expected<std::expected<int, bool> const>);
static_assert(some_expected<std::expected<int, bool> &>);
static_assert(some_expected<std::expected<int, bool> const &>);
static_assert(some_expected<std::expected<int, bool> &&>);
static_assert(some_expected<std::expected<int, bool> const &&>);

static_assert(some_expected_void<std::expected<void, bool>>);
static_assert(some_expected_void<std::expected<void, bool> const>);
static_assert(some_expected_void<std::expected<void, bool> &>);
static_assert(some_expected_void<std::expected<void, bool> const &>);
static_assert(some_expected_void<std::expected<void, bool> &&>);
static_assert(some_expected_void<std::expected<void, bool> const &&>);
static_assert(not some_expected_void<std::expected<int, bool>>);
static_assert(not some_expected_void<std::expected<int, bool> const>);
static_assert(not some_expected_void<std::expected<int, bool> &>);
static_assert(not some_expected_void<std::expected<int, bool> const &>);
static_assert(not some_expected_void<std::expected<int, bool> &&>);
static_assert(not some_expected_void<std::expected<int, bool> const &&>);

static_assert(some_expected_non_void<std::expected<int, bool>>);
static_assert(some_expected_non_void<std::expected<int, bool> const>);
static_assert(some_expected_non_void<std::expected<int, bool> &>);
static_assert(some_expected_non_void<std::expected<int, bool> const &>);
static_assert(some_expected_non_void<std::expected<int, bool> &&>);
static_assert(some_expected_non_void<std::expected<int, bool> const &&>);
static_assert(not some_expected_non_void<std::expected<void, bool>>);
static_assert(not some_expected_non_void<std::expected<void, bool> const>);
static_assert(not some_expected_non_void<std::expected<void, bool> &>);
static_assert(not some_expected_non_void<std::expected<void, bool> const &>);
static_assert(not some_expected_non_void<std::expected<void, bool> &&>);
static_assert(not some_expected_non_void<std::expected<void, bool> const &&>);

namespace {
struct Error {};
struct Xerror final : Error {};
struct Value final {};
} // namespace

static_assert(some_optional<std::optional<int>>);
static_assert(some_optional<std::optional<int> const>);
static_assert(some_optional<std::optional<int> &>);
static_assert(some_optional<std::optional<int> const &>);
static_assert(some_optional<std::optional<int> &&>);
static_assert(some_optional<std::optional<int> const &&>);

static_assert(some_monadic_type<std::expected<int, bool>>);
static_assert(some_monadic_type<std::expected<int, bool> const>);
static_assert(some_monadic_type<std::expected<int, bool> &>);
static_assert(some_monadic_type<std::expected<int, bool> const &>);
static_assert(some_monadic_type<std::expected<int, bool> &&>);
static_assert(some_monadic_type<std::expected<int, bool> const &&>);
static_assert(some_monadic_type<std::optional<int>>);
static_assert(some_monadic_type<std::optional<int> const>);
static_assert(some_monadic_type<std::optional<int> &>);
static_assert(some_monadic_type<std::optional<int> const &>);
static_assert(some_monadic_type<std::optional<int> &&>);
static_assert(some_monadic_type<std::optional<int> const &&>);

// clang-format off
static_assert(same_kind<std::optional<bool>, std::optional<Value>>);
static_assert(not same_kind<std::optional<bool>, std::expected<void, bool>>);
static_assert(not same_kind<std::optional<int>, std::expected<int, Error>>);
static_assert(not same_kind<std::optional<Error>, std::expected<void, Error>>);
static_assert(same_kind<std::expected<Value, Error>, std::expected<void, Error>>);
static_assert(same_kind<std::expected<void, Error>, std::expected<void, Error>>);
static_assert(same_kind<std::expected<void, Error>, std::expected<int, Error>>);
static_assert(same_kind<std::expected<int, Error>, std::expected<void, Error>>);
static_assert(same_kind<std::expected<int, Error>, std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error>, std::expected<Value, Error>>);
static_assert(not same_kind<std::expected<Value, Error>, std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error>, std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error>, std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error>, std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<int, Error>, std::expected<Value, Xerror>>);
static_assert(not same_kind<std::expected<void, Error>, std::expected<Value, Xerror>>);

static_assert(same_kind<std::optional<int>          , std::optional<Value>>);
static_assert(same_kind<std::optional<int>          , std::optional<Value> const>);
static_assert(same_kind<std::optional<int>          , std::optional<Value> &>);
static_assert(same_kind<std::optional<int>          , std::optional<Value> const &>);
static_assert(same_kind<std::optional<int>          , std::optional<Value> &&>);
static_assert(same_kind<std::optional<int>          , std::optional<Value> const &&>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value>>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value> const>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value> &>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value> const &>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value> &&>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value> const &&>);
static_assert(same_kind<std::optional<int> const    , std::optional<Value>>);
static_assert(same_kind<std::optional<int> &        , std::optional<Value>>);
static_assert(same_kind<std::optional<int> &        , std::optional<Value> const>);
static_assert(same_kind<std::optional<int> &        , std::optional<Value> &>);
static_assert(same_kind<std::optional<int> &        , std::optional<Value> const &>);
static_assert(same_kind<std::optional<int> &        , std::optional<Value> &&>);
static_assert(same_kind<std::optional<int> &        , std::optional<Value> const &&>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value>>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value> const>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value> &>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value> const &>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value> &&>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value> const &&>);
static_assert(same_kind<std::optional<int> const &  , std::optional<Value>>);
static_assert(same_kind<std::optional<int> &&       , std::optional<Value>>);
static_assert(same_kind<std::optional<int> &&       , std::optional<Value> const>);
static_assert(same_kind<std::optional<int> &&       , std::optional<Value> &>);
static_assert(same_kind<std::optional<int> &&       , std::optional<Value> const &>);
static_assert(same_kind<std::optional<int> &&       , std::optional<Value> &&>);
static_assert(same_kind<std::optional<int> &&       , std::optional<Value> const &&>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value>>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value> const>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value> &>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value> const &>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value> &&>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value> const &&>);
static_assert(same_kind<std::optional<int> const && , std::optional<Value>>);

static_assert(same_kind<std::expected<int, Error>          , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<int, Error>          , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<int, Error>          , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<int, Error>          , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<int, Error>          , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<int, Error>          , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<int, Error> const    , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<int, Error> const    , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<int, Error> const    , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<int, Error> const    , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<int, Error> const    , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<int, Error> const    , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<int, Error> &        , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<int, Error> &        , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<int, Error> &        , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<int, Error> &        , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<int, Error> &        , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<int, Error> &        , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<int, Error> const &  , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<int, Error> const &  , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<int, Error> const &  , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<int, Error> const &  , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<int, Error> const &  , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<int, Error> const &  , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<int, Error> &&       , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<int, Error> &&       , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<int, Error> &&       , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<int, Error> &&       , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<int, Error> &&       , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<int, Error> &&       , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<int, Error> const && , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<int, Error> const && , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<int, Error> const && , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<int, Error> const && , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<int, Error> const && , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<int, Error> const && , std::expected<Value, Error> const &&>);

static_assert(not same_kind<std::expected<int, Error>          , std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error>          , std::expected<int, Xerror> const>);
static_assert(not same_kind<std::expected<int, Error>          , std::expected<int, Xerror> &>);
static_assert(not same_kind<std::expected<int, Error>          , std::expected<int, Xerror> const &>);
static_assert(not same_kind<std::expected<int, Error>          , std::expected<int, Xerror> &&>);
static_assert(not same_kind<std::expected<int, Error>          , std::expected<int, Xerror> const &&>);
static_assert(not same_kind<std::expected<int, Error> const    , std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error> const    , std::expected<int, Xerror> const>);
static_assert(not same_kind<std::expected<int, Error> const    , std::expected<int, Xerror> &>);
static_assert(not same_kind<std::expected<int, Error> const    , std::expected<int, Xerror> const &>);
static_assert(not same_kind<std::expected<int, Error> const    , std::expected<int, Xerror> &&>);
static_assert(not same_kind<std::expected<int, Error> const    , std::expected<int, Xerror> const &&>);
static_assert(not same_kind<std::expected<int, Error> &        , std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error> &        , std::expected<int, Xerror> const>);
static_assert(not same_kind<std::expected<int, Error> &        , std::expected<int, Xerror> &>);
static_assert(not same_kind<std::expected<int, Error> &        , std::expected<int, Xerror> const &>);
static_assert(not same_kind<std::expected<int, Error> &        , std::expected<int, Xerror> &&>);
static_assert(not same_kind<std::expected<int, Error> &        , std::expected<int, Xerror> const &&>);
static_assert(not same_kind<std::expected<int, Error> const &  , std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error> const &  , std::expected<int, Xerror> const>);
static_assert(not same_kind<std::expected<int, Error> const &  , std::expected<int, Xerror> &>);
static_assert(not same_kind<std::expected<int, Error> const &  , std::expected<int, Xerror> const &>);
static_assert(not same_kind<std::expected<int, Error> const &  , std::expected<int, Xerror> &&>);
static_assert(not same_kind<std::expected<int, Error> const &  , std::expected<int, Xerror> const &&>);
static_assert(not same_kind<std::expected<int, Error> &&       , std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error> &&       , std::expected<int, Xerror> const>);
static_assert(not same_kind<std::expected<int, Error> &&       , std::expected<int, Xerror> &>);
static_assert(not same_kind<std::expected<int, Error> &&       , std::expected<int, Xerror> const &>);
static_assert(not same_kind<std::expected<int, Error> &&       , std::expected<int, Xerror> &&>);
static_assert(not same_kind<std::expected<int, Error> &&       , std::expected<int, Xerror> const &&>);
static_assert(not same_kind<std::expected<int, Error> const && , std::expected<int, Xerror>>);
static_assert(not same_kind<std::expected<int, Error> const && , std::expected<int, Xerror> const>);
static_assert(not same_kind<std::expected<int, Error> const && , std::expected<int, Xerror> &>);
static_assert(not same_kind<std::expected<int, Error> const && , std::expected<int, Xerror> const &>);
static_assert(not same_kind<std::expected<int, Error> const && , std::expected<int, Xerror> &&>);
static_assert(not same_kind<std::expected<int, Error> const && , std::expected<int, Xerror> const &&>);

static_assert(same_kind<std::expected<void, Error>          , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error>          , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<void, Error>          , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<void, Error>          , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<void, Error>          , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<void, Error>          , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<void, Error> const    , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error> const    , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<void, Error> const    , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<void, Error> const    , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<void, Error> const    , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<void, Error> const    , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<void, Error> &        , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error> &        , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<void, Error> &        , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<void, Error> &        , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<void, Error> &        , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<void, Error> &        , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<void, Error> const &  , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error> const &  , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<void, Error> const &  , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<void, Error> const &  , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<void, Error> const &  , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<void, Error> const &  , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<void, Error> &&       , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error> &&       , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<void, Error> &&       , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<void, Error> &&       , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<void, Error> &&       , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<void, Error> &&       , std::expected<Value, Error> const &&>);
static_assert(same_kind<std::expected<void, Error> const && , std::expected<Value, Error>>);
static_assert(same_kind<std::expected<void, Error> const && , std::expected<Value, Error> const>);
static_assert(same_kind<std::expected<void, Error> const && , std::expected<Value, Error> &>);
static_assert(same_kind<std::expected<void, Error> const && , std::expected<Value, Error> const &>);
static_assert(same_kind<std::expected<void, Error> const && , std::expected<Value, Error> &&>);
static_assert(same_kind<std::expected<void, Error> const && , std::expected<Value, Error> const &&>);

static_assert(not same_kind<std::expected<void, Error>          , std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error>          , std::expected<void, Xerror> const>);
static_assert(not same_kind<std::expected<void, Error>          , std::expected<void, Xerror> &>);
static_assert(not same_kind<std::expected<void, Error>          , std::expected<void, Xerror> const &>);
static_assert(not same_kind<std::expected<void, Error>          , std::expected<void, Xerror> &&>);
static_assert(not same_kind<std::expected<void, Error>          , std::expected<void, Xerror> const &&>);
static_assert(not same_kind<std::expected<void, Error> const    , std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error> const    , std::expected<void, Xerror> const>);
static_assert(not same_kind<std::expected<void, Error> const    , std::expected<void, Xerror> &>);
static_assert(not same_kind<std::expected<void, Error> const    , std::expected<void, Xerror> const &>);
static_assert(not same_kind<std::expected<void, Error> const    , std::expected<void, Xerror> &&>);
static_assert(not same_kind<std::expected<void, Error> const    , std::expected<void, Xerror> const &&>);
static_assert(not same_kind<std::expected<void, Error> &        , std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error> &        , std::expected<void, Xerror> const>);
static_assert(not same_kind<std::expected<void, Error> &        , std::expected<void, Xerror> &>);
static_assert(not same_kind<std::expected<void, Error> &        , std::expected<void, Xerror> const &>);
static_assert(not same_kind<std::expected<void, Error> &        , std::expected<void, Xerror> &&>);
static_assert(not same_kind<std::expected<void, Error> &        , std::expected<void, Xerror> const &&>);
static_assert(not same_kind<std::expected<void, Error> const &  , std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error> const &  , std::expected<void, Xerror> const>);
static_assert(not same_kind<std::expected<void, Error> const &  , std::expected<void, Xerror> &>);
static_assert(not same_kind<std::expected<void, Error> const &  , std::expected<void, Xerror> const &>);
static_assert(not same_kind<std::expected<void, Error> const &  , std::expected<void, Xerror> &&>);
static_assert(not same_kind<std::expected<void, Error> const &  , std::expected<void, Xerror> const &&>);
static_assert(not same_kind<std::expected<void, Error> &&       , std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error> &&       , std::expected<void, Xerror> const>);
static_assert(not same_kind<std::expected<void, Error> &&       , std::expected<void, Xerror> &>);
static_assert(not same_kind<std::expected<void, Error> &&       , std::expected<void, Xerror> const &>);
static_assert(not same_kind<std::expected<void, Error> &&       , std::expected<void, Xerror> &&>);
static_assert(not same_kind<std::expected<void, Error> &&       , std::expected<void, Xerror> const &&>);
static_assert(not same_kind<std::expected<void, Error> const && , std::expected<void, Xerror>>);
static_assert(not same_kind<std::expected<void, Error> const && , std::expected<void, Xerror> const>);
static_assert(not same_kind<std::expected<void, Error> const && , std::expected<void, Xerror> &>);
static_assert(not same_kind<std::expected<void, Error> const && , std::expected<void, Xerror> const &>);
static_assert(not same_kind<std::expected<void, Error> const && , std::expected<void, Xerror> &&>);
static_assert(not same_kind<std::expected<void, Error> const && , std::expected<void, Xerror> const &&>);
// clang-format on
} // namespace fn
