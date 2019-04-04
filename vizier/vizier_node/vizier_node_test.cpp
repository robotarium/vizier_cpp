#include "vizier/vizier_node/vizier_node.h"
#include "nlohmann/json.h"
#include "gtest/gtest.h"
#include <iostream>


TEST(CreateMessageId, CreatesCorrectIdLength) {
    auto s = size_t(64);
    for(int i = 0; i < 100; ++i) {
        EXPECT_EQ(vizier::create_message_id().length(), s);
    }
}

json create_response(std::string status, json body, std::string topic_type) {
    return {{"status", std::move(status)}, {"body", std::move(body)}, {"type", std::move(topic_type)}};
}
TEST(CreateResponse, CreatesCorrectResponse) {
    json body = {
        {"msg", 1}
    };

    create_response("1", body, "DATA");
}

