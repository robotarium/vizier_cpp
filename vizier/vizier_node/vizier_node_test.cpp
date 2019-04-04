#include "vizier/vizier_node/vizier_node.h"
#include "nlohmann/json.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <tuple>

using json = nlohmann::json;
const int FUZZY_LENGTH = 100;

TEST(CreateMessageId, CreatesCorrectIdLength) {
    auto expected = size_t(64);
    for(int i = 0; i < FUZZY_LENGTH; ++i) {
        EXPECT_EQ(expected, vizier::create_message_id().length());
    }
}

TEST(CreateResponse, CreatesCorrectResponse) {
    json body = {
        {"msg", 1}
    };

    json expected = {
        {"status", "1"},
        {"body", body},
        {"type", "DATA"}
    };

    json expected2 = {
        {"status", "2"},
        {"body", body},
        {"type", "STREAM"}
    };

    EXPECT_EQ(expected, vizier::create_response("1", body, "DATA"));
    EXPECT_EQ(expected2, vizier::create_response("2", body, "STREAM"));
}

TEST(CreateResponse, CreatesCorrectResponseFuzzy) {
    for(int i = 0; i < FUZZY_LENGTH; ++i) {

        auto status = vizier::random_string(FUZZY_LENGTH, vizier::rand_char);
        auto type = vizier::random_string(FUZZY_LENGTH, vizier::rand_char);

        json body = {
            {"msg", 1}
        };

        json expected = {
            {"status", status},
            {"body", body},
            {"type", type}
        };

        EXPECT_EQ(expected, vizier::create_response(status, body, type));
    }
}

TEST(ParseNodeDescriptor, Basic) {
    json descriptor = {

        {"endpoint", "node"},
        {
            "links", 
            {
                {"/0", {{"type", "STREAM"}}}
            } 
        },
        {"requests", ""}
    };

    std::unordered_map<std::string, std::string> expected = {
        {"node/0", "STREAM"}
    };

    std::unordered_map<std::string, std::string> result;
    bool ok;
    std::tie(result, ok) = vizier::parse_descriptor(descriptor);

    EXPECT_TRUE(ok);
    EXPECT_EQ(expected, result);
}