#include "vizier/vizier_node/utils.h"
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

    EXPECT_EQ(expected, vizier::create_response("1", body, vizier::LinkType::DATA));
    EXPECT_EQ(expected2, vizier::create_response("2", body, vizier::LinkType::STREAM));
}

TEST(CreateResponse, CreatesCorrectResponseFuzzy) {
    for(int i = 0; i < FUZZY_LENGTH; ++i) {

        auto status = vizier::random_string(FUZZY_LENGTH, vizier::rand_char);
        auto type = "DATA";

        json body = {
            {"msg", 1}
        };

        json expected = {
            {"status", status},
            {"body", body},
            {"type", type}
        };

        EXPECT_EQ(expected, vizier::create_response(status, body, vizier::LinkType::DATA));
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

    std::unordered_map<std::string, vizier::LinkType> expected = {
        {"node/0", vizier::LinkType::STREAM}
    };

    std::unordered_map<std::string, vizier::LinkType> result;
    bool ok;
    std::tie(result, ok) = vizier::parse_descriptor(descriptor);

    EXPECT_TRUE(ok);
    EXPECT_EQ(expected, result);
}

TEST(ParseNodeDescriptor, Intermediate) {
    json descriptor = {

        {"endpoint", "node"},
        {
            "links", 
            {
                {"/0", {{"type", "STREAM"}}},
                {"/1", 
                    {
                        {"links", 
                            {
                                {"/2", {{"type", "DATA"}}}
                            }
                        }
                    }
                }
            } 
        },
        {"requests", ""}
    };

    std::unordered_map<std::string, vizier::LinkType> expected = {
        {"node/0", vizier::LinkType::STREAM},
        {"node/1/2", vizier::LinkType::DATA}
    };

    std::unordered_map<std::string, vizier::LinkType> result;
    bool ok;
    std::tie(result, ok) = vizier::parse_descriptor(descriptor);

    EXPECT_TRUE(ok);
    EXPECT_EQ(expected, result);
} 

TEST(GetRequestsFromDesriptor, EmptyRequests) {
    json descriptor = {
        {"requests", {}}
    };

    std::vector<vizier::RequestData> expected;

    auto result = vizier::get_requests_from_descriptor(descriptor);

    EXPECT_EQ(expected, result);
}

TEST(GetRequestsFromDesriptor, NonemptyRequests) {

    //std::vector<std::string> expected = {"1/test", "2/test"};
    std::vector<vizier::RequestData> expected = {{"1/test", false, vizier::LinkType::DATA}, {"2/test", true, vizier::LinkType::STREAM}};
    json descriptor;
    descriptor["requests"] = {
        {
            {"link", "1/test"},
            {"type", "DATA"},
            {"required", false}
        },
        {
            {"link", "2/test"},
            {"type", "STREAM"},
            {"required", true}
        },
    };

    auto result = vizier::get_requests_from_descriptor(descriptor);
    EXPECT_EQ(expected, result);
}

TEST(GetRequestsFromDesriptor, NoRequests) {

    std::vector<vizier::RequestData> expected;

    json descriptor = {};
    auto result = vizier::get_requests_from_descriptor(descriptor);
    EXPECT_EQ(expected, result);
}