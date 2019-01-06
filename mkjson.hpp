// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKJSON_HPP
#define MEASUREMENT_KIT_MKJSON_HPP

/// @file mkjson.hpp. This file contains code that simplifies dealing with
/// parsing and serializing JSON messages such as the messages exchanged by
/// Measurement Kit's FFI API.

#include "json.hpp"
#include "mkdata.hpp"

namespace mk {
namespace json {

/// parse parses @p input into @p json. Returns true on success and false
/// on failure. On failure, @p error contains the error that occurred.
bool parse(const std::string &input, nlohmann::json &json,
           std::string &error) noexcept;

/// serialize is like parse except that it serializes.
bool serialize(const nlohmann::json &json, std::string &output,
               std::string &error) noexcept;

/// SwapResult is the result of a swap_value_at operation.
enum class SwapResult {
  /// success indicates success.
  success,

  /// bad_json_pointer indicates that the JSON pointer format is bad.
  bad_json_pointer,

  /// not_found indicates that no element was found.
  not_found,

  /// cast_failed indicates that the element has not the expected type.
  cast_failed
};

/// swap_value_at<@p Type> swaps @p value with the value of the JSON element in
/// @p json pointed by @p pointer. Returns a SwapResult enumeration.
///
/// This template function is typically used to parse an incoming JSON
/// document. It should work for the following @p Type types:
///
/// - bool
/// - double
/// - int64_t
/// - std::map<std::string, nlohmann::json>
/// - std::string
/// - std::vector<nlohmann::json>
///
/// There are template specializations for std::map<std::string, std::string>
/// and std::vector<std::string>, which are two common cases.
template <typename Type>
SwapResult swap_value_at(
    nlohmann::json &json, const std::string &pointer, Type &value) noexcept {
  nlohmann::json::json_pointer p;
  try {
    p = nlohmann::json::json_pointer{pointer};
  } catch (const std::exception &) {
    return SwapResult::bad_json_pointer;
  }
  nlohmann::json element;
  try {
    std::swap(json.at(p), element);
  } catch (const std::exception &) {
    return SwapResult::not_found;
  }
  Type *vp = element.get_ptr<Type *>();
  if (vp == nullptr) {
    return SwapResult::cast_failed;
  }
  std::swap(*vp, value);
  return SwapResult::success;
}

/// swap_value_at<std::vector<std::string>> is a template specialization
/// for the std::vector<std::string> common case.
template <>
SwapResult swap_value_at(
    nlohmann::json &json, const std::string &pointer,
    std::vector<std::string> &value) noexcept {
  std::vector<nlohmann::json> vec;
  SwapResult res = swap_value_at(json, pointer, vec);
  if (res != SwapResult::success) {
    return res;
  }
  value.clear();
  for (auto &elem : vec) {
    std::string s;
    if ((res = swap_value_at(elem, "", s)) != SwapResult::success) {
      return res;
    }
    value.push_back(std::move(s));
  }
  return SwapResult::success;
}

/// swap_value_at<std::map<std::string, std::string>> is a template
/// specialization for the std::map<std::string, std::string> common case.
template <>
SwapResult swap_value_at(
    nlohmann::json &json, const std::string &pointer,
    std::map<std::string, std::string> &value) noexcept {
  std::map<std::string, nlohmann::json> map;
  SwapResult res = swap_value_at(json, pointer, map);
  if (res != SwapResult::success) {
    return res;
  }
  value.clear();
  for (auto &elem : map) {
    std::string s;
    if ((res = swap_value_at(elem.second, "", s)) != SwapResult::success) {
      return res;
    }
    value.emplace(std::make_pair(std::move(elem.first), std::move(s)));
  }
  return SwapResult::success;
}

/// MoveinResult is the result of an insert operation.
enum class MoveinResult {
  /// success indicates success.
  success,

  /// bad_json_pointer indicates that the JSON pointer format is bad.
  bad_json_pointer,

  /// cannot_create indicates that an element cannot be created
  /// for a specified JSON pointer.
  cannot_create
};

template <typename Type>
MoveinResult movein_(
    nlohmann::json &json, const std::string &pointer, Type &&value) noexcept {
  nlohmann::json::json_pointer p;
  try {
    p = nlohmann::json::json_pointer{pointer};
  } catch (const std::exception &) {
    return MoveinResult::bad_json_pointer;
  }
  try {
    json[p] = std::move(value);
  } catch (const std::exception &) {
    return MoveinResult::cannot_create;
  }
  return MoveinResult::success;
}

/// movein<@p Type> moves @p value in the JSON element pointed by @p pointer
/// in the @p json document. Returns an enumeration indicating whether we
/// succeeded or what error has occurred.
///
/// We have template specializations for types containing strings.
template <typename Type>
MoveinResult movein(
    nlohmann::json &json, const std::string &pointer, Type &&value) noexcept {
  return movein_(json, pointer, std::move(value));
}

/// movein<std::string> is a specialized movein for strings.
template <>
MoveinResult movein(
    nlohmann::json &json, const std::string &pointer,
    std::string &&value) noexcept {
  if (!mk::data::contains_valid_utf8(value)) {
    value = mk::data::base64_encode(std::move(value));
  }
  return movein_(json, pointer, std::move(value));
}

/// movein<std::vector<std::string>> is a specialized movein for vectors
/// containing only strings (a common case).
template <>
MoveinResult movein(
    nlohmann::json &json, const std::string &pointer,
    std::vector<std::string> &&value) noexcept {
  for (std::string &s : value) {
    if (!mk::data::contains_valid_utf8(s)) {
      s = mk::data::base64_encode(std::move(s));
    }
  }
  return movein_(json, pointer, std::move(value));
}

/// movein<std::map<std::string, std::string>> is a specialized movein for maps
/// containing only strings (a common case).
template <>
MoveinResult movein(
    nlohmann::json &json, const std::string &pointer,
    std::map<std::string, std::string> &&value) noexcept {
  for (auto &pair : value) {
    if (!mk::data::contains_valid_utf8(pair.second)) {
      pair.second = mk::data::base64_encode(std::move(pair.second));
    }
  }
  return movein_(json, pointer, std::move(value));
}

}  // namespace json
}  // namespace mk

// MKJSON_INLINE_IMPL allows to inline the implementation.
#ifdef MKJSON_INLINE_IMPL

namespace mk {
namespace json {

bool parse(const std::string &input, nlohmann::json &json,
           std::string &error) noexcept {
  try {
    json = nlohmann::json::parse(input);
  } catch (const std::exception &exc) {
    error = exc.what();
    return false;
  }
  return true;
}

bool serialize(const nlohmann::json &json, std::string &output,
               std::string &error) noexcept {
  try {
    output = json.dump();
  } catch (const std::exception &exc) {
    error = exc.what();
    return false;
  }
  return true;
}

}  // namespace json
}  // namespace mk
#endif  // MKJSON_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKJSON_HPP
