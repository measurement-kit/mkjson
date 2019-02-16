// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKJSON_HPP
#define MEASUREMENT_KIT_MKJSON_HPP

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

namespace mk {
namespace json {

/// Result contains the result of an operation.
template <typename Type>
class Result {
 public:
  /// good indicates whether the operation succeeded.
  bool good = true;

  /// failure indicates why the operation failed.
  std::string failure;

  /// value is the result of a successful operation.
  Type value = {};
};

// Result<void> is a template specialization for the void special case.
template <>
class Result<void> {
 public:
  bool good = true;
  std::string failure;
};

/// JSON is a JSON value.
class JSON {
 public:
  /// parse parses @p json_str and returns the result.
  static Result<JSON> parse(const std::string &json_str) noexcept;

  /// dump serializes the JSON and returns the result.
  Result<std::string> dump() const noexcept;

  /// JSON creates a new null JSON.
  JSON() noexcept;

  /// JSON is not copy constructible.
  JSON(const JSON &) = delete;

  /// operator= is not allowed for copy operations.
  JSON &operator=(const JSON &) = delete;

  /// JSON is move constructible.
  JSON(JSON &&) noexcept;

  /// operator= is allowed for move operations.
  JSON &operator=(JSON &&) noexcept;

  /// is_array tells you whether the JSON is an array.
  bool is_array() const noexcept;

  /// is_boolean tells you whether the JSON is a boolean.
  bool is_boolean() const noexcept;

  /// is_float64 tells you whether the JSON is a float64.
  bool is_float64() const noexcept;

  /// is_int64 tells you whether the JSON is a int64.
  bool is_int64() const noexcept;

  /// is_null tells you whether the JSON is null.
  bool is_null() const noexcept;

  /// is_object tells you whether the JSON is an object.
  bool is_object() const noexcept;

  /// is_string tells you whether the JSON is a string.
  bool is_string() const noexcept;

  /// get_value_at assumes that the JSON is an object and removes the value
  /// currently at @p key, returning it. This method has move semantics; after
  /// it has successfully returned, no value will be at @p key anymore.
  Result<JSON> get_value_at(const std::string &key) noexcept;

  /// get_value_array assumes that the JSON is an array and returns such
  /// array. This method has move semantics; after it successfully returns,
  /// the JSON will become empty.
  Result<std::vector<JSON>> get_value_array() noexcept;

  /// get_value_boolean is like get_value_array but for boolean.
  Result<bool> get_value_boolean() noexcept;

  /// get_value_float64 is like get_value_array but for float64.
  Result<double> get_value_float64() noexcept;

  /// get_value_int64 is like get_value_array but for int64.
  Result<int64_t> get_value_int64() noexcept;

  /// get_value_string is like get_value_array but for string.
  Result<std::string> get_value_string() noexcept;

  /// set_value_at is the dual operation of get_value_at.
  Result<void> set_value_at(const std::string &key, JSON &&value) noexcept;

  /// set_value_array unconditionally sets the JSON value to be @p value. The
  /// previous content of the JSON will be wiped.
  void set_value_array(std::vector<JSON> &&value) noexcept;

  /// set_value_float64 is like set_value_array but for float64.
  void set_value_float64(double value) noexcept;

  /// set_value_int64 is like set_value_array but for int64.
  void set_value_int64(int64_t value) noexcept;

  /// set_value_string is like set_value_array but for strings.
  void set_value_string(std::string &&value) noexcept;

  /// ~JSON destroys the allocated resources.
  ~JSON() noexcept;

  // Friend is a forward declaration to a friend class.
  class Friend;

  // Friend is a friend of us.
  friend class Friend;

 private:
  // Impl is a forward declaration to the internal implementation.
  class Impl;

  // JSON constructs an instance from an implementation.
  explicit JSON(Impl &&other_impl) noexcept;

  // impl is a unique pointer to the internal implementation.
  std::unique_ptr<Impl> impl;
};

}  // namespace json
}  // namespace mk

// MKJSON_INLINE_IMPL allows to inline the implementation.
#ifdef MKJSON_INLINE_IMPL

#include <exception>
#include <type_traits>
#include <utility>

#include "json.hpp"
#include "mkdata.hpp"

namespace mk {
namespace json {

// JSON::Impl is the concrete implementation of JSON.
class JSON::Impl {
 public:
  // nlohmann_json is the underlying nlohmann/json instance.
  nlohmann::json nlohmann_json;

  // Impl constructs the implementation from an existing JSON.
  explicit Impl(nlohmann::json &&value) noexcept;

  // Impl constructs an empty implementation.
  Impl() noexcept;
};

/*explicit*/ JSON::Impl::Impl(nlohmann::json &&value) noexcept {
  std::swap(value, nlohmann_json);
}

JSON::Impl::Impl() noexcept {}

// JSON::Friend is the definition of the class friend of JSON.
class JSON::Friend {
 public:
  // unwrap allows to unwrap a JSON to get the inner nlohmann::json.
  static nlohmann::json &unwrap(JSON &json) noexcept;
};

/*static*/ nlohmann::json &JSON::Friend::unwrap(JSON &json) noexcept {
  return json.impl->nlohmann_json;
}

/*explicit*/ JSON::JSON(Impl &&other_impl) noexcept : JSON{} {
  std::swap(other_impl, *impl);
}

/*static*/ Result<JSON> JSON::parse(const std::string &json_str) noexcept {
  Result<JSON> result;
  try {
    result.value.impl->nlohmann_json = nlohmann::json::parse(json_str);
  } catch (const std::exception &exc) {
    result.good = false;
    result.failure = exc.what();
  }
  return result;
}

Result<std::string> JSON::dump() const noexcept {
  Result<std::string> result;
  try {
    result.value = impl->nlohmann_json.dump();
  } catch (const std::exception &exc) {
    result.good = false;
    result.failure = exc.what();
  }
  return result;
}

JSON::JSON() noexcept { impl.reset(new JSON::Impl); }

JSON::JSON(JSON &&other) noexcept : JSON{} {
  std::swap(impl, other.impl);
}

JSON &JSON::operator=(JSON &&other) noexcept {
  std::swap(impl, other.impl);
  return *this;
}

bool JSON::is_array() const noexcept {
  return impl->nlohmann_json.is_array();
}

bool JSON::is_boolean() const noexcept {
  return impl->nlohmann_json.is_boolean();
}

bool JSON::is_float64() const noexcept {
  return impl->nlohmann_json.is_number_float();
}

bool JSON::is_int64() const noexcept {
  return impl->nlohmann_json.is_number_integer();
}

bool JSON::is_null() const noexcept {
  return impl->nlohmann_json.is_null();
}

bool JSON::is_object() const noexcept {
  return impl->nlohmann_json.is_object();
}

bool JSON::is_string() const noexcept {
  return impl->nlohmann_json.is_string();
}

Result<JSON> JSON::get_value_at(const std::string &key) noexcept {
  Result<JSON> result;
  try {
    result.value.impl->nlohmann_json = std::move(impl->nlohmann_json.at(key));
    impl->nlohmann_json.erase(key);
  } catch (const std::exception &exc) {
    result.good = false;
    result.failure = exc.what();
  }
  return result;
}

Result<std::vector<JSON>> JSON::get_value_array() noexcept {
  Result<std::vector<JSON>> result;
  auto valuep = impl->nlohmann_json.get_ptr<std::vector<nlohmann::json> *>();
  if (valuep == nullptr) {
    result.good = false;
    result.failure = "Not an array";
    return result;
  }
  for (nlohmann::json &entry : *valuep) {
    result.value.push_back(JSON{JSON::Impl{std::move(entry)}});
  }
  impl->nlohmann_json = nullptr;
  return result;
}

Result<bool> JSON::get_value_boolean() noexcept {
  Result<bool> result;
  auto valuep = impl->nlohmann_json.get_ptr<bool *>();
  if (valuep == nullptr) {
    result.good = false;
    result.failure = "Not a boolean";
    return result;
  }
  result.value = *valuep;
  impl->nlohmann_json = nullptr;
  return result;
}

Result<double> JSON::get_value_float64() noexcept {
  Result<double> result;
  auto valuep = impl->nlohmann_json.get_ptr<double *>();
  if (valuep == nullptr) {
    result.good = false;
    result.failure = "Not a float64";
    return result;
  }
  result.value = *valuep;
  impl->nlohmann_json = nullptr;
  return result;
}

Result<int64_t> JSON::get_value_int64() noexcept {
  Result<int64_t> result;
  auto valuep = impl->nlohmann_json.get_ptr<int64_t *>();
  if (valuep == nullptr) {
    result.good = false;
    result.failure = "Not an int64";
    return result;
  }
  result.value = *valuep;
  impl->nlohmann_json = nullptr;
  return result;
}

Result<std::string> JSON::get_value_string() noexcept {
  Result<std::string> result;
  auto valuep = impl->nlohmann_json.get_ptr<std::string *>();
  if (valuep == nullptr) {
    result.good = false;
    result.failure = "Not a string";
    return result;
  }
  std::swap(result.value, *valuep);
  impl->nlohmann_json = nullptr;
  return result;
}

Result<void> JSON::set_value_at(const std::string &key, JSON &&value) noexcept {
  Result<void> result;
  try {
    std::swap(value.impl->nlohmann_json, impl->nlohmann_json[key]);
  } catch (const std::exception &exc) {
    result.good = false;
    result.failure = exc.what();
  }
  return result;
}

void JSON::set_value_array(std::vector<JSON> &&value) noexcept {
  std::vector<nlohmann::json> array;
  for (JSON &entry : value) {
    array.push_back(std::move(entry.impl->nlohmann_json));
  }
  impl->nlohmann_json = std::move(array);
}

void JSON::set_value_float64(double value) noexcept {
  impl->nlohmann_json = value;
}

void JSON::set_value_int64(int64_t value) noexcept {
  impl->nlohmann_json = value;
}

void JSON::set_value_string(std::string &&value) noexcept {
  if (!mk::data::contains_valid_utf8(value)) {
    value = mk::data::base64_encode(std::move(value));
  }
  impl->nlohmann_json = std::move(value);
}

JSON::~JSON() noexcept {}

}  // namespace json
}  // namespace mk
#endif  // MKJSON_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKJSON_HPP
