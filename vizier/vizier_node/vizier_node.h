#ifndef VIZIER_VIZIER_NODE_VIZIER_NODE
#define VIZIER_VIZIER_NODE_VIZIER_NODE

#include "nlohmann/json.hpp"
#include "vizier/vizier_node/utils.h"
#include "spdlog/spdlog.h"
#include "vizier/utils/mqttclient/mqttclient_async.h"
#include <unordered_set>

using json = nlohmann::json;

namespace vizier {

class VizierNode {
private:
    const std::string host_;
    const int port_;
    const json descriptor_;
    std::string request_link_;
    mutable MqttClientAsync mqtt_client_;

    std::unordered_map<std::string, LinkType> expanded_links_;
    std::unordered_map<std::string, LinkType> requests_;
    //std::vector<std::string> requests_;
    std::unordered_set<std::string> puttable_links;
    std::unordered_set<std::string> publishable_links;
    std::unordered_set<std::string> gettable_links;
    std::unordered_set<std::string> subscribable_links;

public:
    VizierNode(const std::string& host, const int port, const json& descriptor) 
    : 
    host_(host),
    port_(port),
    descriptor_(descriptor),
    mqtt_client_(host, port) 
    {
        bool ok;
        std::tie(this->expanded_links_, ok) = parse_descriptor(descriptor);
        if(!ok) {
            spdlog::error("Invalid node descriptor");
            throw(std::runtime_error("Invalid node descriptor"));
        }

        // On which links can we publish?
        for(const std::pair<std::string, LinkType>& item : this->expanded_links_) {
           if(item.second == LinkType::DATA) {
               this->puttable_links.insert(item.first);
           } 

           if(item.second == LinkType::STREAM) {
               this->publishable_links.insert(item.first);
           }
        }

        this->requests_ = get_requests_from_descriptor(descriptor);
    } 
};

} // namespace vizier

#endif