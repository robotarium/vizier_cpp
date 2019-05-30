#include "vizier/vizier_node/vizier_node.h"
#include "nlohmann/json.hpp"
#include <string>
#include <chrono>
#include <thread>

using json = nlohmann::json;

int main() {
    std::string host = "192.168.1.8";
    int port = 1884;
    
    json descriptor = {
        {"endpoint", "dummy"},
        {
            "links", 
            {
                {"/0", {{"type", "STREAM"}}},
                {"/1", {{"type", "DATA"}}}
            } 
        },
        {"requests", {}}
    };

    vizier::VizierNode node(host, port, descriptor);
    node.put("dummy/1", "data");
    
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}