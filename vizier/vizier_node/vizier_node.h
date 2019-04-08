#ifndef VIZIER_VIZIER_NODE_VIZIER_NODE
#define VIZIER_VIZIER_NODE_VIZIER_NODE

#include "nlohmann/json.hpp"
#include "vizier/vizier_node/utils.h"
#include "spdlog/spdlog.h"
#include "vizier/utils/mqttclient/mqttclient_async.h"

using json = nlohmann::json;
using namespace vizier;

class VizierNode {
private:
    const std::string host_;
    const int port_;
    json descriptor_;
    std::string request_link_;

    MqttClientAsync mqtt_client_;
    std::unordered_map<std::string, std::string> expanded_links_;
    std::vector<std::string> requests_;

public:
    VizierNode(const std::string& host, const int port, const json& descriptor) 
    : 
    host_(host),
    port_(port),
    descriptor_(descriptor),
    mqtt_client_(host, port)
    {
        //this->port_ = port;
        //this->host_ = host;
        //this->descriptor_ = descriptor;

        bool ok;
        std::tie(this->expanded_links_, ok) = parse_descriptor(descriptor);
        if(!ok) {
            spdlog::error("Invalid node descriptor");
            throw(std::runtime_error("Invalid node descriptor"));
        }

        this->requests_ = get_requests_from_descriptor(descriptor);
    } 
};

#endif