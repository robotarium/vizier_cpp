#ifndef VIZIER_VIZIER_NODE_VIZIER_NODE
#define VIZIER_VIZIER_NODE_VIZIER_NODE

#include "nlohmann/json.hpp"
#include "vizier/vizier_node/utils.h"
#include "spdlog/spdlog.h"
#include "vizier/utils/mqttclient/mqttclient_async.h"
#include "vizier/utils/tsqueue/tsqueue.h"
#include <unordered_set>
#include <optional>
#include <memory>

#include <iostream>


namespace vizier {

using json = nlohmann::json;
using string = std::string;

template<class T> using optional = std::optional<T>;
template<class T> using vector = std::vector<T>;
template<class T, class U> using unordered_map = std::unordered_map<T, U>;
template<class T> using unordered_set = std::unordered_set<T>;
template<class T> using shared_ptr = std::shared_ptr<T>;


class VizierNode {
private:

    const string host_;
    const int port_;
    const json descriptor_;
    string request_link_;
    MqttClientAsync mqtt_client_;

    string endpoint_;
    unordered_map<string, LinkType> expanded_links_;
    vector<RequestData> requests_;
    unordered_set<string> puttable_links_;
    unordered_set<string> publishable_links_;
    unordered_set<string> gettable_links_;
    unordered_set<string> subscribable_links_;
    unordered_map<string, string> link_data_;

public:

    /*
        TODO: Doc
    */
    VizierNode(const string& host, const int port, const json& descriptor) 
    : 
    host_(host),
    port_(port),
    descriptor_(descriptor),
    mqtt_client_(host, port)
    {
        if(this->descriptor_.count("endpoint") == 0) {
            string er = "Descriptor must contain key 'endpoint'";
            spdlog::error(er);
            
            throw std::runtime_error(er);
        }

        this->endpoint_ = this->descriptor_["endpoint"];

        auto result = parse_descriptor(descriptor_);

        if(!result) {
            string er = "Invalid node descriptor";
            spdlog::error(er);

            throw std::runtime_error(er);
        }

        this->expanded_links_ = result.value();

        // On which links can we publish?
        for(const std::pair<string, LinkType>& item : this->expanded_links_) {
           if(item.second == LinkType::DATA) {
               this->puttable_links_.insert(item.first);
           } 

           if(item.second == LinkType::STREAM) {
               this->publishable_links_.insert(item.first);
           }
        }

        // endpoint/node_descriptor is a reserved link!
        string reserved = this->endpoint_ + "/node_descriptor";
        if(this->puttable_links_.find(reserved) != this->puttable_links_.end()) {
            spdlog::error("Reserved link: " + reserved + " found in puttable links.  Deleting");
            this->puttable_links_.erase(reserved);
        }

        // Set up remote links
        auto get_req_result = get_requests_from_descriptor(descriptor_);

        if(!get_req_result) {
            string er = "Descriptor requests invalid";
            spdlog::error(er);

            throw std::runtime_error(er);
        }

        this->requests_ = get_req_result.value(); 
        for(const auto& r : this->requests_) {
            if(r.type == LinkType::DATA) {
                this->gettable_links_.insert(r.link);
            }

            if(r.type == LinkType::STREAM) {
                this->subscribable_links_.insert(r.link);
            }
        }
    }

    optional<string> make_get_request(const string& link, size_t retries, std::chrono::milliseconds timeout) {
        string id = create_message_id();
        json get_request = create_request(id, Methods::GET, link, {});

        size_t found = link.find_first_of('/');

        if(found == string::npos) {
            spdlog::error("Invalid link {0}.  Requires a '/'");
            return std::nullopt;
        }

        string remote_node = link.substr(0, found);
        string response_link = create_response_link(remote_node, id);
        string request_link = create_request_link(remote_node);

        std::cout << response_link << std::endl;

        optional<shared_ptr<ThreadSafeQueue<string>>> maybe_q = this->mqtt_client_.subscribe(response_link);

        if(!maybe_q) {
            return std::nullopt;
        }

        auto q = maybe_q.value();
        optional<string> message;

        for(size_t i = 0; i < retries; ++i) {
            this->mqtt_client_.async_publish(request_link, get_request.dump());
            std::cout << get_request << std::endl;
            spdlog::info("Sending GET request...");
            message = q->dequeue(timeout);

            if(!message) {
                spdlog::info("Request timed out.  Retrying");
                continue;
            } else {
                break;
            }
        }

        if(!message) {
            return std::nullopt;
        }

        std::cout << message.value() << std::endl;
        json json_message = json::parse(message.value());
        std::cout << json_message << std::endl;

        if(json_message.count("body") == 0) {
            spdlog::error("GET response received with no body");
            return std::nullopt;
        }

        return json_message["body"];
    }

    /*
        TODO: Doc
    */
    void publish(const string& link, string message) {
        if(this->publishable_links_.find(link) == this->publishable_links_.end()) {
            spdlog::error("Cannot publish on link {0} because it has not been declared as a link of type STREAM", link);
            return;
        }

        this->mqtt_client_.async_publish(link, std::move(message));
    }

    /*
        TODO: Doc
    */
    string get(const string& link) {
        if(this->gettable_links_.find(link) == this->gettable_links_.end()) {
            spdlog::error("Cannot get on link {0} because it has not been declared as a request of type DATA", link);
            return "";
        }

        return "";
    }

    /*
        TODO: Doc
    */
    optional<ThreadSafeQueue<string>> subscribe(const string& link) {
        if(this->subscribable_links_.find(link) == this->subscribable_links_.end()) {
            spdlog::error("Cannot get on link {0} because it has not been declared as a request of type STREAM", link);
            return std::nullopt;
        }

        return {};
    }

    /*
        TODO: Doc
    */
    void put(const string& link, string data) {
        if(this->puttable_links_.find(link) == this->subscribable_links_.end()) {
           spdlog::error("Cannot put on link {0} because it has not been declared as a link of type DATA", link);
           return; 
        }

        // TODO: IMPLEMENT
    }
};

} // namespace vizier

#endif
