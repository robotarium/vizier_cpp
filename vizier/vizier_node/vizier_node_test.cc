#include "nlohmann/json.hpp"
#include "vizier/vizier_node/vizier_node.h"
#include "gtest/gtest.h"
#include <chrono>

using json = nlohmann::json;

TEST(VizierNode, Dummy) {
    std::string host = "192.168.1.8";
    json descriptor = {};
    vizier::VizierNode(host, 1884, descriptor);
    EXPECT_EQ(0, 0);
}

TEST(VizierNode, Get) {
    json descriptor = {
        {"endpoint", "node"},
        {
            "links", 
            {
                {"/0", {{"type", "STREAM"}}}
            } 
        },
        {"requests", {}}
    };
    std::string host = "192.168.1.8";

    bool connected = true;
    std::unique_ptr<vizier::VizierNode> node;
    node = std::make_unique<vizier::VizierNode>(host, 1884, descriptor);

    EXPECT_TRUE(connected);

    if(!connected) {
        return;
    }

    auto result = node->make_get_request("node/node_descriptor", 5, std::chrono::milliseconds(1000));
}