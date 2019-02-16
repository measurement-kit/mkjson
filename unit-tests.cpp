#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define MKDATA_INLINE_IMPL
#include "mkdata.hpp"

#include "mkjson.hpp"

#include <iostream>
#include <type_traits>

using namespace mk::json;

TEST_CASE("parse works as expected") {
  SECTION("for a valid JSON") {
    Result<JSON> result = JSON::parse(R"({"success": true})");
    REQUIRE(result.good);
  }

  SECTION("for an invalid JSON") {
    Result<JSON> result = JSON::parse(R"({"success": true,})");
    REQUIRE(!result.good);
  }
}

TEST_CASE("serialize works as expected") {
  JSON json;
  Result<void> dump = json.dump();
  REQUIRE(dump.good);
  std::clog << dump.value << std::endl;
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

// TODO(bassosimone): add more checks and verify move semantics

TEST_CASE("get_value_at works as expected") {
  Result<JSON> doc = JSON::parse(R"({"success": true})");
  REQUIRE(doc.good);

  SECTION("in the common case") {
    Result<JSON> e = doc.value.get_value_at("success");
    REQUIRE(e.good);
    REQUIRE(e.value.is_boolean());
  }

  SECTION("when the key is missing") {
    Result<JSON> e = doc.value.get_value_at("failure");
    REQUIRE(!e.good);
  }

  SECTION("when the JSON is not an object") {
    doc.value.set_value_int64(0);
    Result<JSON> e = doc.value.get_value_at("success");
    REQUIRE(!e.good);
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
  }

  SECTION("for a non array") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<std::vector<JSON>> array = doc.value.get_value_array();
    REQUIRE(!array.good);
  }
}

TEST_CASE("get_value_boolean works as expected") {
  SECTION("for a valid boolean") {
    Result<JSON> doc = JSON::parse("true");
    REQUIRE(doc.good);
    Result<bool> boolean = doc.value.get_value_boolean();
    REQUIRE(boolean.good);
  }

  SECTION("for a non boolean") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<bool> boolean = doc.value.get_value_boolean();
    REQUIRE(!boolean.good);
  }
}

TEST_CASE("get_value_float64 works as expected") {
  SECTION("for a valid float64") {
    Result<JSON> doc = JSON::parse("3.14");
    REQUIRE(doc.good);
    Result<double> float64 = doc.value.get_value_float64();
    REQUIRE(float64.good);
  }

  SECTION("for a non float64") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<double> float64 = doc.value.get_value_float64();
    REQUIRE(!float64.good);
  }
}

TEST_CASE("get_value_int64 works as expected") {
  SECTION("for a valid int64") {
    Result<JSON> doc = JSON::parse("314");
    REQUIRE(doc.good);
    Result<int64_t> int64 = doc.value.get_value_int64();
    REQUIRE(int64.good);
  }

  SECTION("for a non int64") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<int64_t> int64 = doc.value.get_value_int64();
    REQUIRE(!int64.good);
  }
}

TEST_CASE("get_value_string works as expected") {
  SECTION("for a valid string") {
    Result<JSON> doc = JSON::parse(R"("hello, world")");
    REQUIRE(doc.good);
    Result<std::string> string = doc.value.get_value_string();
    REQUIRE(string.good);
  }

  SECTION("for a non string") {
    Result<JSON> doc = JSON::parse("{}");
    REQUIRE(doc.good);
    Result<std::string> string = doc.value.get_value_string();
    REQUIRE(!string.good);
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
