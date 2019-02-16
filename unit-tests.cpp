#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define MKDATA_INLINE_IMPL
#include "mkdata.hpp"

#define MKJSON_INLINE_IMPL
#include "mkjson.hpp"

#include <iostream>
#include <type_traits>

using namespace mk::json;

TEST_CASE("parse works as expected") {
  SECTION("for a valid JSON") {
    Result<JSON> result = JSON::parse(R"({"success": true})");
    REQUIRE(result.good);
    REQUIRE(result.failure.size() == 0);
  }

  SECTION("for an invalid JSON") {
    Result<JSON> result = JSON::parse(R"({)");
    REQUIRE(!result.good);
    REQUIRE(result.failure.size() > 0);
    std::clog << result.failure << std::endl;
  }
}

// clang-format off
const uint8_t binary_input[] = {
  0x57, 0xe5, 0x79, 0xfb, 0xa6, 0xbb, 0x0d, 0xbc, 0xce, 0xbd, 0xa7, 0xa0,
  0xba, 0xa4, 0x78, 0x78, 0x12, 0x59, 0xee, 0x68, 0x39, 0xa4, 0x07, 0x98,
  0xc5, 0x3e, 0xbc, 0x55, 0xcb, 0xfe, 0x34, 0x3c, 0x7e, 0x1b, 0x5a, 0xb3,
  0x22, 0x9d, 0xc1, 0x2d, 0x6e, 0xca, 0x5b, 0xf1, 0x10, 0x25, 0x47, 0x1e,
  0x44, 0xe2, 0x2d, 0x60, 0x08, 0xea, 0xb0, 0x0a, 0xcc, 0x05, 0x48, 0xa0,
  0xf5, 0x78, 0x38, 0xf0, 0xdb, 0x3f, 0x9d, 0x9f, 0x25, 0x6f, 0x89, 0x00,
  0x96, 0x93, 0xaf, 0x43, 0xac, 0x4d, 0xc9, 0xac, 0x13, 0xdb, 0x22, 0xbe,
  0x7a, 0x7d, 0xd9, 0x24, 0xa2, 0x52, 0x69, 0xd8, 0x89, 0xc1, 0xd1, 0x57,
  0xaa, 0x04, 0x2b, 0xa2, 0xd8, 0xb1, 0x19, 0xf6, 0xd5, 0x11, 0x39, 0xbb,
  0x80, 0xcf, 0x86, 0xf9, 0x5f, 0x9d, 0x8c, 0xab, 0xf5, 0xc5, 0x74, 0x24,
  0x3a, 0xa2, 0xd4, 0x40, 0x4e, 0xd7, 0x10, 0x1f
};
// clang-format on

TEST_CASE("serialize works as expected") {
  SECTION("for a valid JSON") {
    JSON json;
    Result<std::string> result = json.dump();
    REQUIRE(result.good);
    REQUIRE(result.failure.size() == 0);
    std::clog << result.value << std::endl;
  }

  SECTION("for an invalid JSON") {
    JSON json;
    nlohmann::json &inner = JSON::Friend::unwrap(json);
    inner = std::string{(char *)binary_input, sizeof(binary_input)};
    Result<std::string> result = json.dump();
    REQUIRE(!result.good);
    REQUIRE(result.failure.size() > 0);
    REQUIRE(result.value.size() == 0);
    std::clog << result.failure << std::endl;
  }
}

TEST_CASE("the default constructor works as expected") {
  JSON json;
  REQUIRE(json.is_null());
}

TEST_CASE("the JSON is not copy constructible") {
  REQUIRE(!std::is_copy_constructible<JSON>());
}

TEST_CASE("the JSON is not copy assignable") {
  REQUIRE(!std::is_copy_assignable<JSON>());
}

TEST_CASE("the JSON is move constructible") {
  REQUIRE(std::is_move_constructible<JSON>());
}

TEST_CASE("the JSON is move assignable") {
  REQUIRE(std::is_move_assignable<JSON>());
}

TEST_CASE("is_array works as expected") {
  Result<JSON> result = JSON::parse("[1, 2, 3]");
  REQUIRE(result.good);
  REQUIRE(result.value.is_array());
}

TEST_CASE("is_boolean works as expected") {
  Result<JSON> result = JSON::parse("true");
  REQUIRE(result.good);
  REQUIRE(result.value.is_boolean());
}

TEST_CASE("is_float64 works as expected") {
  Result<JSON> result = JSON::parse("1.234567");
  REQUIRE(result.good);
  REQUIRE(result.value.is_float64());
}

TEST_CASE("is_int64 works as expected") {
  Result<JSON> result = JSON::parse("1234567");
  REQUIRE(result.good);
  REQUIRE(result.value.is_int64());
}

TEST_CASE("is_null works as expected") {
  Result<JSON> result = JSON::parse("null");
  REQUIRE(result.good);
  REQUIRE(result.value.is_null());
}

TEST_CASE("is_object works as expected") {
  Result<JSON> result = JSON::parse(R"({"success": true})");
  REQUIRE(result.good);
  REQUIRE(result.value.is_object());
}

TEST_CASE("is_string works as expected") {
  Result<JSON> result = JSON::parse(R"("success")");
  REQUIRE(result.good);
  REQUIRE(result.value.is_string());
}

TEST_CASE("get_value_at works as expected") {
  Result<JSON> doc = JSON::parse(R"({"success": true})");
  REQUIRE(doc.good);

  SECTION("in the common case") {
    Result<JSON> e = doc.value.get_value_at("success");
    REQUIRE(e.good);
    REQUIRE(e.value.is_boolean());
    nlohmann::json &inner = JSON::Friend::unwrap(e.value);
    REQUIRE(inner.count("success") <= 0);
  }

  SECTION("when the key is missing") {
    Result<JSON> e = doc.value.get_value_at("failure");
    REQUIRE(!e.good);
    REQUIRE(e.failure.size() > 0);
    std::clog << e.failure << std::endl;
  }

  SECTION("when the JSON is not an object") {
    doc.value.set_value_int64(0);
    Result<JSON> e = doc.value.get_value_at("success");
    REQUIRE(!e.good);
    REQUIRE(e.failure.size() > 0);
    std::clog << e.failure << std::endl;
  }
}

TEST_CASE("get_value_array works as expected") {
  SECTION("for a valid array") {
    Result<JSON> doc = JSON::parse("[1, 2, 3, 4]");
    REQUIRE(doc.good);
    Result<std::vector<JSON>> array = doc.value.get_value_array();
    REQUIRE(array.good);
    for (auto &e : array.value) {
      REQUIRE(e.is_int64());
    }
    REQUIRE(doc.value.is_null());
  }

  SECTION("for a non array") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<std::vector<JSON>> array = doc.value.get_value_array();
    REQUIRE(!array.good);
    REQUIRE(array.failure.size() > 0);
    std::clog << array.failure << std::endl;
  }
}

TEST_CASE("get_value_boolean works as expected") {
  SECTION("for a valid boolean") {
    Result<JSON> doc = JSON::parse("true");
    REQUIRE(doc.good);
    Result<bool> boolean = doc.value.get_value_boolean();
    REQUIRE(boolean.good);
    REQUIRE(doc.value.is_null());
  }

  SECTION("for a non boolean") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<bool> boolean = doc.value.get_value_boolean();
    REQUIRE(!boolean.good);
    REQUIRE(boolean.failure.size() > 0);
    std::clog << boolean.failure << std::endl;
  }
}

TEST_CASE("get_value_float64 works as expected") {
  SECTION("for a valid float64") {
    Result<JSON> doc = JSON::parse("3.14");
    REQUIRE(doc.good);
    Result<double> float64 = doc.value.get_value_float64();
    REQUIRE(float64.good);
    REQUIRE(doc.value.is_null());
  }

  SECTION("for a non float64") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<double> float64 = doc.value.get_value_float64();
    REQUIRE(!float64.good);
    REQUIRE(float64.failure.size() > 0);
    std::clog << float64.failure << std::endl;
  }
}

TEST_CASE("get_value_int64 works as expected") {
  SECTION("for a valid int64") {
    Result<JSON> doc = JSON::parse("314");
    REQUIRE(doc.good);
    Result<int64_t> int64 = doc.value.get_value_int64();
    REQUIRE(int64.good);
    REQUIRE(doc.value.is_null());
  }

  SECTION("for a non int64") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<int64_t> int64 = doc.value.get_value_int64();
    REQUIRE(!int64.good);
    REQUIRE(int64.failure.size() > 0);
    std::clog << int64.failure << std::endl;
  }
}

TEST_CASE("get_value_string works as expected") {
  SECTION("for a valid string") {
    Result<JSON> doc = JSON::parse(R"("hello, world")");
    REQUIRE(doc.good);
    Result<std::string> string = doc.value.get_value_string();
    REQUIRE(string.good);
    REQUIRE(doc.value.is_null());
  }

  SECTION("for a non string") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<std::string> string = doc.value.get_value_string();
    REQUIRE(!string.good);
    REQUIRE(string.failure.size() > 0);
    std::clog << string.failure << std::endl;
  }
}

TEST_CASE("set_value_at works as expected") {
  Result<JSON> v = JSON::parse("false");
  REQUIRE(v.good);

  SECTION("when the key already exists") {
    Result<JSON> doc = JSON::parse(R"({"success": true})");
    REQUIRE(doc.good);
    Result<void> res = doc.value.set_value_at("success", std::move(v.value));
    REQUIRE(res.good);
  }

  SECTION("when the key does not exist") {
    Result<JSON> doc = JSON::parse(R"({"success": true})");
    REQUIRE(doc.good);
    Result<void> res = doc.value.set_value_at("failure", std::move(v.value));
    REQUIRE(res.good);
  }

  SECTION("when the JSON is not an object") {
    Result<JSON> doc = JSON::parse("0");
    REQUIRE(doc.good);
    Result<void> res = doc.value.set_value_at("success", std::move(v.value));
    REQUIRE(!res.good);
    REQUIRE(res.failure.size() > 0);
    std::clog << res.failure << std::endl;
  }
}

TEST_CASE("we can successfully create a complex JSON") {
  JSON document;
  {
    std::vector<JSON> vector;
    {
      JSON number;
      number.set_value_int64(42);
      vector.push_back(std::move(number));
    }
    {
      JSON pi;
      pi.set_value_float64(3.1415);
      vector.push_back(std::move(pi));
    }
    {
      JSON name;
      name.set_value_string("Simone");
      vector.push_back(std::move(name));
    }
    JSON array;
    array.set_value_array(std::move(vector));
    Result<void> result = document.set_value_at("array", std::move(array));
    REQUIRE(result.good);
  }
  {
    JSON number;
    number.set_value_int64(42);
    Result<void> result = document.set_value_at("number", std::move(number));
    REQUIRE(result.good);
  }
  Result<std::string> dump = document.dump();
  REQUIRE(dump.good);
  std::clog << dump.value << std::endl;
}
