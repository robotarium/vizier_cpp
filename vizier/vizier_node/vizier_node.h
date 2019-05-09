#ifndef VIZIER_VIZIER_NODE_VIZIER_NODE
#define VIZIER_VIZIER_NODE_VIZIER_NODE

#include "nlohmann/json.hpp"
#include "vizier/vizier_node/utils.h"
#include "spdlog/spdlog.h"
#include "vizier/utils/mqttclient/mqttclient_async.h"
#include <unordered_set>

using json = nlohmann::json;
using string = std::string;

namespace vizier {

class VizierNode {
private:
    const std::string host_;
    const int port_;
    const json descriptor_;
    std::string request_link_;
    mutable MqttClientAsync mqtt_client_;

    string endpoint_;
    std::unordered_map<std::string, LinkType> expanded_links_;
    std::vector<vizier::RequestData> requests_;
    std::unordered_set<std::string> puttable_links_;
    std::unordered_set<std::string> publishable_links_;
    std::unordered_set<std::string> gettable_links_;
    std::unordered_set<std::string> subscribable_links_;

    std::string make_request() {
        return "";
    }

public:
    VizierNode(const std::string& host, const int port, const json& descriptor) 
    : 
    host_(host),
    port_(port),
    descriptor_(descriptor),
    mqtt_client_(host, port) 
    {

        if(descriptor.count("endpoint") == 0) {
            throw(std::runtime_error("Descriptor must contain endpoint key"));
        }

        this->endpoint_ = descriptor.count("endpoint");

        bool ok;

        std::tie(this->expanded_links_, ok) = parse_descriptor(descriptor);

        if(!ok) {
            spdlog::error("Invalid node descriptor");
            throw(std::runtime_error("Invalid node descriptor"));
        }

        // On which links can we publish?
        for(const std::pair<std::string, LinkType>& item : this->expanded_links_) {
           if(item.second == LinkType::DATA) {
               this->puttable_links_.insert(item.first);
           } 

           if(item.second == LinkType::STREAM) {
               this->publishable_links_.insert(item.first);
           }
        }

        string reserved = this->endpoint_ + "/" + "node_descriptor";
        if(this->puttable_links_.find(reserved) != this->puttable_links_.end()) {
            spdlog::error("Reserved link: " + reserved + " found in puttable links.  Deleting");
            this->puttable_links_.erase(this->endpoint_+"/node_descriptor");
        }

        // Set up remote links
        this->requests_ = get_requests_from_descriptor(descriptor);
        for(const auto& r : this->requests_) {
            if(r.type == LinkType::DATA) {
                this->gettable_links_.insert(r.link);
            }

            if(r.type == LinkType::STREAM) {
                this->subscribable_links_.insert(r.link);
            }
        } 
    } 
};

} // namespace vizier

#endif
