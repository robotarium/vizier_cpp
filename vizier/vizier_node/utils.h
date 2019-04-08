#ifndef VIZIER_VIZIER_NODE_UTILS
#define VIZIER_VIZIER_NODE_UTILS

#include "nlohmann/json.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <functional> // for std::function
#include <algorithm>  // for std::generate_n

namespace vizier {

using json = nlohmann::json;

namespace {
    std::vector<char> charset() {
        return std::vector<char>( 
        {'0','1','2','3','4',
            '5','6','7','8','9',
            'A','B','C','D','E','F',
            'G','H','I','J','K',
            'L','M','N','O','P',
            'Q','R','S','T','U',
            'V','W','X','Y','Z',
            'a','b','c','d','e','f',
            'g','h','i','j','k',
            'l','m','n','o','p',
            'q','r','s','t','u',
            'v','w','x','y','z'
        });
    }   

    //  Create a non-deterministic random number generator.
    const auto ch_set = charset();
    std::random_device seed;
    std::default_random_engine rng(seed());
    //  Create a random number "shaper" that will give us uniformly distributed indices into the character set
    std::uniform_int_distribution<> dist(0, ch_set.size()-1);

    char rand_char() {
        return ch_set[dist(rng)];
    }

    std::string random_string(size_t length, std::function<char(void)> rand_char) {
        std::string str(length, 0);
        std::generate_n(str.begin(), length, rand_char);
        return str;
    }
} // anon namespace

//  Creates a unique message ID for a request
//  
//  Returns:
//      A secure, random 64-byte message ID
std::string create_message_id() {
    return random_string(64, rand_char);
}

//  Creates a GET response JSON message
//  
//  Returns:
//      A JSON object containing the keys: status, body, type
json create_response(std::string status, json body, std::string topic_type) {
    // TODO: Convert to const& ?
    return {{"status", std::move(status)}, {"body", std::move(body)}, {"type", std::move(topic_type)}};
}

//  Creates a response link for a node
// 
//  Returns:
//      The response link for the node
std::string create_response_link(std::string node, std::string message_id) {
   return std::move(node) + "/" + "responses/" + std::move(message_id);
}

//  Creates a request link for a node
//
//  Returns:
//      The request link for a node
std::string create_request_link(std::string node) {
    return std::move(node) + "/requests";
}

//  Creates a JSON-encoded GET request
//  
//  Args:
//      id: id for the request.  Should be large and random
//      method: must be GET || PUT
//      link: link on which to make the request
//      body: body for the request.  
//
//  Returns:
//      A JSON object representing the request with the keys: id, method, link, body
json create_request(std::string id, std::string method, std::string link, json body) {
    return {{"id", std::move(id)}, 
            {"method", std::move(method)}, 
            {"link", std::move(link)}, 
            {"body", std::move(body)}};
}

//  Returns true if path is a subpath of link.
//
//  Args:
//      link: the superset
//      path: the subset
//
//  Returns:
//      True if path is a subset of link and false otherwise
bool is_subpath_of(const std::string& link, const std::string& path) {

    // Path has to be a subset of link, so it can't be longer
    if(path.length() > link.length()) {
        return false;
    }

    // Edge case
    if(path.length() == 0) {
        return true;
    }

    return link.substr(0, path.length()) == path;
}

std::string to_absolute_path(std::string base, std::string path) {

    if(path.length() == 0) {
        return base;
    }

    if(path[0] == '/') {
        return std::move(base) + std::move(path);
    } else {
        return path;
    }
}

std::pair<std::unordered_map<std::string, std::string>, bool> parse_descriptor(const std::string& path, const std::string& link, const json& descriptor) {

    auto link_here = to_absolute_path(path, link);

    if(!is_subpath_of(link_here, path)) {
           return {{}, false};
    }

    if(descriptor.count("links") == 0 || descriptor["links"].size() == 0) {
        if(descriptor.count("type") == 1) {
            std::pair<std::unordered_map<std::string, std::string>, bool> ret;
            ret.first = {{link_here, descriptor["type"]}};
            ret.second = true;
            return ret;
        } else {
            // This is an error.  Leaf links must contain a type
            return{{}, false};
        }
    } else {
        // Links is nonempty
        std::unordered_map<std::string, std::string> parsed_links;

        for(const auto& item : descriptor["links"].items()) {
            auto pl = parse_descriptor(link_here, item.key(), item.value());

            if(!pl.second) {
                return {{}, false};
            }

            parsed_links.insert(pl.first.begin(), pl.first.end());
        }

        return {parsed_links, true};
    }
}

std::pair<std::unordered_map<std::string, std::string>, bool> parse_descriptor(const json& descriptor) {

    if(descriptor.count("endpoint") == 0) {
        return {{}, false};
    }

    return parse_descriptor("", descriptor["endpoint"], descriptor); 
}

std::vector<std::string> get_requests_from_descriptor(const json& descriptor) {

    if(descriptor.count("requests") == 0) {
        return {};
    }

    std::vector<std::string> ret;
    auto req = descriptor["requests"];

    // Will throw if not a string
    for(const auto& item : req) {
       ret.push_back(item);
    }

    return ret;
}

} // vizier namespace

#endif