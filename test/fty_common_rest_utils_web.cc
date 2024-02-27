/*
 *
 * Copyright (C) 2015 - 2020 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file fty_common_rest_utils_web.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Arnaud Quette <ArnaudQuette@Eaton.com>
 * \brief Not yet documented file
 */

#include "fty_common_rest_utils_web.h"
#include <catch2/catch.hpp>
#include <fty_common_json.h>
#include <fty_common_macros.h>
#include <cxxtools/serializationinfo.h>

#include <tnt/httpreply.h>
#include <sstream>

TEST_CASE("single parameter ('inttype') invocation")
{
    int           var_int           = -1;
    long int      var_long_int      = -2;
    long long int var_long_long_int = -3;
    short         var_short         = 4;
    int32_t       var_int32_t       = 6;
    int64_t       var_int64_t       = 7;
    byte          var_byte          = 8;

    unsigned int           var_unsigned_int           = 10;
    unsigned long int      var_unsigned_long_int      = 20;
    unsigned long long int var_unsigned_long_long_int = 30;
    unsigned short         var_unsigned_short         = 40;
    uint32_t               var_uint32_t               = 50;
    uint64_t               var_uint64_t               = 60;


    std::string x = utils::json::jsonify(var_int);
    CHECK(x.compare(std::to_string(var_int)) == 0);

    x = utils::json::jsonify(var_long_int);
    CHECK(x.compare(std::to_string(var_long_int)) == 0);

    x = utils::json::jsonify(var_long_long_int);
    CHECK(x.compare(std::to_string(var_long_long_int)) == 0);

    x = utils::json::jsonify(var_short);
    CHECK(x.compare(std::to_string(var_short)) == 0);

    x = utils::json::jsonify(var_int32_t);
    CHECK(x.compare(std::to_string(var_int32_t)) == 0);

    x = utils::json::jsonify(var_int64_t);
    CHECK(x.compare(std::to_string(var_int64_t)) == 0);

    x = utils::json::jsonify(var_byte);
    CHECK(x.compare(std::to_string(var_byte)) == 0);

    x = utils::json::jsonify(var_unsigned_int);
    CHECK(x.compare(std::to_string(var_unsigned_int)) == 0);

    x = utils::json::jsonify(var_unsigned_long_int);
    CHECK(x.compare(std::to_string(var_unsigned_long_int)) == 0);

    x = utils::json::jsonify(var_unsigned_long_long_int);
    CHECK(x.compare(std::to_string(var_unsigned_long_long_int)) == 0);

    x = utils::json::jsonify(var_unsigned_short);
    CHECK(x.compare(std::to_string(var_unsigned_short)) == 0);

    x = utils::json::jsonify(var_uint32_t);
    CHECK(x.compare(std::to_string(var_uint32_t)) == 0);

    x = utils::json::jsonify(var_uint64_t);
    CHECK(x.compare(std::to_string(var_uint64_t)) == 0);
}

TEST_CASE("single parameter ('string') invocation")
{
    const char*  const_char = "*const char with a '\"' quote and newline \n '\\\"'";
    std::string  str        = const_char;
    std::string& str_ref    = str;
    std::string* str_ptr    = &str;

    std::string x = utils::json::jsonify(const_char);
    CHECK(x.compare("\"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\"") == 0);

    x = utils::json::jsonify(str);
    CHECK(x.compare("\"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\"") == 0);

    x = utils::json::jsonify(str_ref);
    CHECK(x.compare("\"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\"") == 0);

    x = utils::json::jsonify(*str_ptr);
    CHECK(x.compare("\"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\"") == 0);
}

TEST_CASE("single parameter ('iterable standard container') invocation")
{
    std::vector<std::string> vector_str{"j\nedna", "dva", "tri"};
    std::vector<int>         vector_int{1, 2, 3};

    std::list<std::string> list_str{"styri", "p\"at", "sest", "sedem"};
    std::list<int>         list_int{4, -5, 6, 7};

    std::string x = utils::json::jsonify(vector_str);
    CHECK(x.compare(R"([ "j\\nedna", "dva", "tri" ])") == 0);

    x = utils::json::jsonify(vector_int);
    CHECK(x.compare(R"([ 1, 2, 3 ])") == 0);

    x = utils::json::jsonify(list_str);
    CHECK(x.compare(R"([ "styri", "p\"at", "sest", "sedem" ])") == 0);

    x = utils::json::jsonify(list_int);
    CHECK(x.compare(R"([ 4, -5, 6, 7 ])") == 0);
}

TEST_CASE("pairs")
{
    const char*              const_char = "*const char with a '\"' quote and newline \n '\\\"'";
    std::string              str        = const_char;
    std::string&             str_ref    = str;
    std::string*             str_ptr    = &str;
    std::vector<std::string> vector_str{"j\nedna", "dva", "tri"};
    std::vector<int>         vector_int{1, 2, 3};
    int64_t                  var_int64_t  = 7;
    uint64_t                 var_uint64_t = 60;

    std::string x = utils::json::jsonify(*str_ptr, var_int64_t);
    CHECK(x.compare(std::string("\"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\" : ") +
                    std::to_string(var_int64_t)) == 0);

    x = utils::json::jsonify("hey\"!\n", str_ref);
    CHECK(x.compare("\"hey\\\"!\\\\n\" : \"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\"") == 0);

    x = utils::json::jsonify(-6, -7);
    CHECK(x.compare(R"("-6" : -7)") == 0);

    x = utils::json::jsonify(var_uint64_t, str);
    CHECK(x.compare(std::string("\"") + std::to_string(var_uint64_t) +
                    "\" : " + "\"*const char with a '\\\"' quote and newline \\\\n '\\\\\\\"'\"") == 0);


    x = utils::json::jsonify("test", vector_str);
    CHECK(x.compare(std::string("\"test\" : ").append(R"([ "j\\nedna", "dva", "tri" ])")) == 0);

    x = utils::json::jsonify(4, vector_int);
    CHECK(x.compare(std::string("\"4\" : ").append("[ 1, 2, 3 ]")) == 0);
}

TEST_CASE("utils::json::create_error_json")
{
    std::string in = "One and two \nthree and \"four\".";

    std::string x = utils::json::create_error_json(in.c_str(), 55);
    CHECK(x.compare("{\n\t\"errors\": [\n\t\t{\n\t\t\t\"message\": \"One and two \\\\nthree and "
                    "\\\"four\\\".\",\n\t\t\t\"code\": 55\n\t\t}\n\t]\n}\n") == 0);

    x = utils::json::create_error_json(in, 56);
    CHECK(x.compare("{\n\t\"errors\": [\n\t\t{\n\t\t\t\"message\": \"One and two \\\\nthree and "
                    "\\\"four\\\".\",\n\t\t\t\"code\": 56\n\t\t}\n\t]\n}\n") == 0);

    std::vector<std::tuple<uint32_t, std::string, std::string>> v;
    v.push_back(std::make_tuple(1, "On\ne", ""));
    v.push_back(std::make_tuple(10, "Tw\"o", ""));

    v.clear();
    v.push_back(std::make_tuple(47, "Received value 'abc'.", ""));
    v.push_back(std::make_tuple(47, "Received value 'def'.", ""));
    v.push_back(std::make_tuple(47, "Received value 'ghi'.", ""));
    x = utils::json::create_error_json(v);
    CHECK(x.compare("{\n\t\"errors\": [\n\t\t{\n\t\t\t\"message\": \"Received value 'abc'.\",\n\t\t\t\"code\": "
                    "47\n\t\t},\n\t\t{\n\t\t\t\"message\": \"Received value 'def'.\",\n\t\t\t\"code\": "
                    "47\n\t\t},\n\t\t{\n\t\t\t\"message\": \"Received value 'ghi'.\",\n\t\t\t\"code\": "
                    "47\n\t\t}\n\t]\n}\n") == 0);
}

TEST_CASE("bad input - std::out_of_range")
{
    try {
        utils::string_to_element_id("0");
        utils::string_to_element_id("-1");
        utils::string_to_element_id("4294967296");
        utils::string_to_element_id("-433838485");
        utils::string_to_element_id("13412342949672");
        utils::string_to_element_id("-4387435873868");
        utils::string_to_element_id(
            "111111111111111111111111111111111111111111111111111111111111111111111111111111111");
        utils::string_to_element_id(
            "-222222222222222222222222222222222222222222222222222222222222222222222222222222222222");
    } catch (std::out_of_range&) {
        // no op, everything is fine
    } catch (std::exception& e) {
        // This will generate a failure in case of other exception
        FAIL(e.what());
    }
}

TEST_CASE("valid input")
{
    try {
        uint32_t r = utils::string_to_element_id("12");
        assert(r == 12);
        r = utils::string_to_element_id("123");
        assert(r == 123);
        r = utils::string_to_element_id("321");
        assert(r == 321);
        r = utils::string_to_element_id("10000");
        assert(r == 10000);
        r = utils::string_to_element_id("131275768");
        assert(r == 131275768);

        r = utils::string_to_element_id("1");
        assert(r == 1);
        r = utils::string_to_element_id("4294967295");
        assert(r == 4294967295);

    } catch (std::exception& e) {
        // This will generate a failure in case of exception
        FAIL(e.what());
    }
}

TEST_CASE("bad input - std::invalid_argument")
{
    try {
        utils::string_to_element_id("");
        utils::string_to_element_id("-");
        utils::string_to_element_id("x");
        utils::string_to_element_id("12s3");
        utils::string_to_element_id("s333");
        utils::string_to_element_id("asf;dguh;8y;34yt83y[Y['8\u6AA6sg ");
    } catch (std::invalid_argument&) {
        // no op, everything is fine
    } catch (std::exception& e) {
        // This will generate a failure in case of other exception
        FAIL(e.what());
    }
}

TEST_CASE("utils::config")
{
    std::mutex                  test_mutex{};
    std::lock_guard<std::mutex> test_lock{test_mutex};

    std::string JSON =
        "{"
        "\"BIOS_SMTP_VERIFY_CA\" : true,"
        "\"BIOS_SMTP_SERVER\" : \"string\","
        "\"BIOS_SMTP_PORT\" : 42,"
        "\"BIOS_SNMP_COMMUNITY_NAME\" : [\"foo\", \"bar\"],"
        "\"config\" : {\"key\" : \"old_array\", \"value\" : [\"old_value1\", \"old_value2\"]}"
        "}";

    cxxtools::SerializationInfo request_doc;
    JSON::readFromString(JSON, request_doc);

    std::map<std::string, zconfig_t*> roots;
    utils::config::json2zpl(roots, request_doc, test_lock);

    zconfig_t* config = roots[utils::config::get_path("BIOS_SMTP_VERIFY_CA")];
    assert(streq(zconfig_get(config, "smtp/verify_ca", "false"), "true"));

    config = roots[utils::config::get_path("BIOS_SNMP_COMMUNITY_NAME")];
    assert(streq(zconfig_get(config, "snmp/community/0", "nullptr"), "foo"));
    assert(streq(zconfig_get(config, "snmp/community/1", "nullptr"), "bar"));

    // legacy_path
    config = roots[utils::config::get_path("old_array")];
    assert(zconfig_get(config, "old_array", nullptr) == nullptr);
    assert(zconfig_locate(config, "old_array") != nullptr);

    assert(streq(zconfig_get(config, "old_array/0", "nullptr"), "old_value1"));
    assert(streq(zconfig_get(config, "old_array/1", "nullptr"), "old_value2"));

    // Cleanup
    utils::config::roots_destroy(roots);
    roots.clear();

    // change value
    std::string JSON2 =
        "{"
        "\"BIOS_SMTP_VERIFY_CA\" : false,"
        "\"BIOS_SNMP_COMMUNITY_NAME\" : [\"ham\", \"spam\"],"
        "\"config\" : {\"key\" : \"old_array\", \"value\" : [\"new_value42\", \"new_value44\"]}"
        "}";

    cxxtools::SerializationInfo request_doc2;
    JSON::readFromString(JSON2, request_doc2);

    utils::config::json2zpl(roots, request_doc2, test_lock);

    config = roots[utils::config::get_path("BIOS_SMTP_VERIFY_CA")];
    assert(streq(zconfig_get(config, "smtp/verify_ca", "true"), "false"));

    config = roots[utils::config::get_path("BIOS_SNMP_COMMUNITY_NAME")];
    assert(streq(zconfig_get(config, "snmp/community/0", "nullptr"), "ham"));
    assert(streq(zconfig_get(config, "snmp/community/1", "nullptr"), "spam"));

    // legacy_path
    config = roots[utils::config::get_path("old_array")];
    assert(!zconfig_get(config, "old_array", nullptr));
    assert(zconfig_locate(config, "old_array") != nullptr);

    assert(streq(zconfig_get(config, "old_array/0", "nullptr"), "new_value42"));
    assert(streq(zconfig_get(config, "old_array/1", "nullptr"), "new_value44"));

    // Cleanup
    utils::config::roots_destroy(roots);
    roots.clear();

    // legacy_path single value
    std::string JSON3 =
        "{ \"config\" : "
        "{\"key\" : \"BIOS_SMTP_VERIFY_CA\", \"value\" : false}"
        "}";

    cxxtools::SerializationInfo request_doc3;
    JSON::readFromString(JSON3, request_doc3);

    utils::config::json2zpl(roots, request_doc3, test_lock);

    config = roots[utils::config::get_path("BIOS_SMTP_VERIFY_CA")];
    assert(streq(zconfig_get(config, "smtp/verify_ca", "true"), "false"));

    // Cleanup
    utils::config::roots_destroy(roots);
    roots.clear();

    // test less values in array
    std::string JSON4 =
        "{"
        "\"BIOS_SNMP_COMMUNITY_NAME\" : [\"eaton\"]"
        "}";

    cxxtools::SerializationInfo request_doc4;
    JSON::readFromString(JSON4, request_doc4);

    utils::config::json2zpl(roots, request_doc4, test_lock);

    config = roots[utils::config::get_path("BIOS_SNMP_COMMUNITY_NAME")];
    zconfig_print(config);
    assert(streq(zconfig_get(config, "snmp/community/0", "nullptr"), "eaton"));
    assert(!zconfig_get(config, "snmp/community/1", nullptr));
    assert(!zconfig_get(config, "snmp/community/2", nullptr));

    // Cleanup
    utils::config::roots_destroy(roots);
    roots.clear();

    printf("OK\n");
}

TEST_CASE("utils::strcmp")
{
    assert (!_strcmp("aa", "bb"));
    assert (!_strcmp("aa", "aaa"));
    assert (!_strcmp("aaa", "aa"));
    assert (_strcmp("aa", "aa"));
    assert (!_strcmp(nullptr, "bb"));
    assert (!_strcmp("aa", nullptr));
    assert (!_strcmp(nullptr, nullptr));
}

TEST_CASE("http_die")
{
    std::function<int(const std::string&)> requestDie = [](const std::string& dieKey) {
        std::ostringstream os;
        tnt::HttpReply reply(os); //http_die inlined

        if (dieKey == "internal-error")
            { http_die("internal-error", TRANSLATE_ME("test_die_reason").c_str()); }
        else if (dieKey == "not-authorized")
            { http_die("not-authorized", TRANSLATE_ME("test_die_reason").c_str()); }
        else if (dieKey == "upstream-err")
            { http_die("upstream-err", TRANSLATE_ME("test_die_reason").c_str()); }

        return int(HTTP_OK);
    };

    struct {
        const std::string dieKey;
        int httpStatus;
    } testVector[] = {
        { "internal-error", HTTP_INTERNAL_SERVER_ERROR },
        { "not-authorized", HTTP_UNAUTHORIZED },
        { "upstream-err", HTTP_BAD_GATEWAY },
        { "no-die", HTTP_OK },
    };

    for (const auto& it : testVector) {
        int httpStatus = requestDie(it.dieKey);
        std::cout << "dieKey: " << it.dieKey << ", httpStatus: " << httpStatus << std::endl;
        CHECK(httpStatus == it.httpStatus);
    }
}
