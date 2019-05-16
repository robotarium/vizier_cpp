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

    auto node = vizier::VizierNode(host, 1884, descriptor);
    bool started = node.start();

    EXPECT_TRUE(started);

    auto result = node.make_get_request("node/node_descriptor", 5, std::chrono::milliseconds(1000));
}