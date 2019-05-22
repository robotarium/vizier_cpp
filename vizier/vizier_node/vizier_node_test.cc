#include "nlohmann/json.hpp"
#include "vizier/vizier_node/vizier_node.h"
#include "gtest/gtest.h"
#include <chrono>

using json = nlohmann::json;

TEST(VizierNode, Construct) {
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

    std::unique_ptr<vizier::VizierNode> node = std::make_unique<vizier::VizierNode>(host, 1884, descriptor);
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

    descriptor["requests"] = {
        {
            {"link", "overhead_tracker/node_descriptor"},
            {"type", "DATA"},
            {"required", false}
        },
        {
            {"link", "dummy/node_descriptor"},
            {"type", "DATA"},
            {"required", false}
        }
    };

    std::string host = "192.168.1.8";

    bool connected = true;
    std::unique_ptr<vizier::VizierNode> node = std::make_unique<vizier::VizierNode>(host, 1884, descriptor);

    EXPECT_TRUE(connected);

    if (!connected) {
        return;
    }

    auto start = std::chrono::steady_clock::now();
    auto result = node->get("overhead_tracker/node_descriptor", 40, std::chrono::milliseconds(500));
    std::cout << "TOOK: " << std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::steady_clock::now()-start)).count() << std::endl;;

    int64_t average = 0;
    for (int i = 0; i < 1000; ++i) {
        start = std::chrono::steady_clock::now();
        result = node->get("dummy/node_descriptor", 40, std::chrono::milliseconds(500));
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::steady_clock::now()-start)).count();
        average += elapsed;
        //std::cout << "TOOK: " << elapsed << std::endl;;
    }

    std::cout << "AVERAGE: " << average / 1000 << std::endl;

    average = 0;
    for (int i = 0; i < 1000; ++i) {
        start = std::chrono::steady_clock::now();
        result = node->get("overhead_tracker/node_descriptor", 40, std::chrono::milliseconds(500));
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::steady_clock::now()-start)).count();
        average += elapsed;
        //std::cout << "TOOK: " << elapsed << std::endl;;
    }

    std::cout << "AVERAGE: " << average / 1000 << std::endl;



    EXPECT_TRUE(bool(result));

    /*if(result) {
        std::cout << json::parse(result.value()) << std::endl;
    }*/
}