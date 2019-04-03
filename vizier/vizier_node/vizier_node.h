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

json create_response(std::string status, json body, std::string topic_type) {
    return {{"status", std::move(status)}, {"body", std::move(body)}, {"type", std::move(topic_type)}};
}

std::string create_response_link(std::string node, std::string message_id) {
   return std::move(node) + "/" + "responses/" + std::move(message_id);
}

std::string create_request_link(std::string node) {
    return std::move(node) + "/requests";
}

json create_request(std::string id, std::string method, std::string link, json body) {
    return {{"id", std::move(id)}, 
            {"method", std::move(method)}, 
            {"link", std::move(link)}, 
            {"body", std::move(body)}};
}

bool is_subpath_of(const std::string& link, const std::string& path) {

    // Path has to be a subset of link, so it can't be longer
    if(path.length() > link.length()) {
        return false;
    }

    return link.substr(path.length()) == path;
}

std::string to_absolute_path(const std::string& base, const std::string& path) {
    if(path.length() == 0) {
        return base;
    }

    if(path[0] == '/') {
        return base + path;
    } else {
        return path;
    }
}

std::pair<std::unordered_map<std::string, std::string>, bool> parse_descriptor(const std::string& path, const std::string& link, json& descriptor) {
    //link_here = combine_paths(path, link);

    if(!is_subpath_of(link, path)) {
           return {{}, false};
    }

    if(descriptor.count("links") == 0 || descriptor["links"].size() == 0) {
        if(descriptor.count("type") == 1) {
            std::pair<std::unordered_map<std::string, std::string>, bool> ret;
            ret.first = {{link, static_cast<std::string>(descriptor["type"])}};
            ret.second = true;
            return ret;
        } else {
            // This is an error.  Leaf links must contain a type
            return{{}, false};
        }
    } else {
        // Links is nonempty
        std::unordered_map<std::string, std::string> parsed_links;
        //auto items = descriptor["links"].items();
        //std::for_each(items.begin(), items.end(), [&path, &link](const auto& item) parse_descriptor(item.key(), item.value()));
        for(const auto& item : descriptor["links"].items()) {
            auto pl = parse_descriptor(path+"/"+link, item.key(), item.value());
            if(!pl.second) {
                return {{}, false};
            }

            parsed_links.insert(pl.first.begin(), pl.first.end());
        }

        return {parsed_links, true};
    }
}

} // vizier namespace