#ifndef VIZIER_VIZIER_NODE_UTILS
#define VIZIER_VIZIER_NODE_UTILS

#include "nlohmann/json.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <functional> // for std::function
#include <algorithm>  // for std::generate_n #include <string>
#include <optional>
#include <spdlog/spdlog.h>


namespace vizier {

using json = nlohmann::json;
using string = std::string;

/* 
    Link types for a node descriptor
*/
enum class LinkType {
    DATA,
    STREAM,
};

/*
    Request types for links
*/
enum class Methods {
    GET,
    PUT,
};

/*
    PoD for link request data
*/
struct RequestData {
    string link = "";
    bool required = false;
    LinkType type = LinkType::STREAM;
};

bool operator== (const RequestData& a, const RequestData& b) {
    return (a.link == b.link) && (a.required == b.required) && (a.type == b.type);
}

template <class T, class U> using unordered_map = std::unordered_map<T, U>;
template<class T> using vector = std::vector<T>;
template<class T> using optional = std::optional<T>;

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

    /*
        TODO: Doc
    */
    string random_string(size_t length, std::function<char(void)> rand_char) {
        string str(length, 0);
        std::generate_n(str.begin(), length, rand_char);
        return str;
    }
} // namespace

/*
    TODO: Doc
*/
string link_type_to_str(const LinkType& type) {
    switch(type) {
        case LinkType::DATA:
            return "DATA";
        break;

        case LinkType::STREAM:
            return "STREAM";
        break;

        default:
            return "DATA";
    };
}

/*
    TODO: Doc
*/
string methods_to_str(const Methods& method) {
    switch(method) {
        case Methods::GET:
            return "GET";
        break;

        case Methods::PUT:
            return "PUT";
        break;

        default:
            return "GET";
    }
}

/*
    TODO: Doc
*/
optional<Methods> string_to_methods(const string& s) {
    if(s == "GET") {
        return Methods::GET;
    }

    if(s == "PUT") {
        return Methods::PUT;
    }

    return std::nullopt;
}

/*
    Creates a unique message ID for a request
  
    Returns:
        A random 64-byte message ID
*/
string create_message_id() {
    return random_string(64, rand_char);
}

/*
    Creates a GET response JSON message
  
    Returns:
        A JSON object containing the keys: status, body, type
*/
json create_response(const string& status, string&& body, const LinkType& topic_type) {
    return {{"status", status}, {"body", body}, {"type", link_type_to_str(topic_type)}};
}

json create_response(const string& status, const string& body, const LinkType& topic_type) {
    return create_response(status, string(body), topic_type);
}

/*  
    Creates a response link for a node
 
    Returns:
        The response link for the node
*/
string create_response_link(const string& node, const string& message_id) {
   return node + "/responses/" + message_id;
}

/*
    Creates a request link for a node

    Returns:
        The request link for a node
*/
string create_request_link(string node) {
    return std::move(node) + "/requests";
}

/* 
    Creates a JSON-encoded GET request
  
    Args:
        id: id for the request.  Should be large and random
        method: must be in Methods enum
        link: link on which to make the request
        body: body for the request.  

    Returns:
        A JSON object representing the request with the keys: id, method, link, body
*/
json create_request(string id, Methods method, string link, json body) {
    // TODO: REMOVE BODY IF METHOD IS PUT
    return {
        {"id", id},
        {"method", methods_to_str(method)}, 
        {"link", std::move(link)}, 
        {"body", std::move(body)}};
}

/*
    Returns true if path is a subpath of link.

    Args:
        link: the superset
        path: the subset

    Returns:
        True if path is a subset of link and false otherwise
*/
bool is_subpath_of(const string& link, const string& path) {

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

/*
    TODO: Doc
*/
string to_absolute_path(string base, string path) {

    if(path.length() == 0) {
        return base;
    }

    if(path[0] == '/') {
        return std::move(base) + std::move(path);
    } else {
        return path;
    }
}

/*
    TODO: Doc
*/
optional<unordered_map<string, LinkType>> parse_descriptor(const string& path, const string& link, const json& descriptor) {

    auto link_here = to_absolute_path(path, link);

    if(!is_subpath_of(link_here, path)) {
        return std::nullopt;
    }

    if(descriptor.count("links") == 0 || descriptor["links"].size() == 0) {
        if(descriptor.count("type") == 1) {

            if(descriptor["type"] == "STREAM") {
                return unordered_map<string, LinkType>({{link_here, LinkType::STREAM}});
            } else if(descriptor["type"] == "DATA") {
                return unordered_map<string, LinkType>({{link_here, LinkType::DATA}});
            } else {
                return std::nullopt;
            }
        } else {
            // This is an error.  Leaf links must contain a type
            return std::nullopt;
        }
    } else {
        // Links is nonempty
        unordered_map<string, LinkType> parsed_links;

        for(const auto& item : descriptor["links"].items()) {
            optional<unordered_map<string, LinkType>> pl = parse_descriptor(link_here, item.key(), item.value());

            if(!pl) {
                return std::nullopt;
            }

            parsed_links.insert(pl->begin(), pl->end()); 
        }

        return parsed_links;
    }
}

/*
    TODO: Doc
*/
optional<unordered_map<string, LinkType>> parse_descriptor(const json& descriptor) {
    if(descriptor.count("endpoint") == 0) {
        spdlog::error("Node descriptor must contain key 'endpoint'");
        return std::nullopt;
    }

    // The case where links is empty here is different from in recursive parsing
    // so we handle it separately.
    if(descriptor.count("links") == 0) {
        return {};
    }

    return parse_descriptor("", descriptor["endpoint"], descriptor);
}

/*
    TODO: Doc
*/
optional<vector<RequestData>> get_requests_from_descriptor(const json& descriptor) {
    if(descriptor.count("requests") == 0) {
        return vector<RequestData>();
    }

    std::vector<RequestData> ret;
    auto req = descriptor["requests"];

    // Will throw if not a string
    for(const auto& item : req) {
        // Each item should be a map with keys link, required, type 

        if(item.count("type") == 0) {
            spdlog::error("Request must contain key 'type'");
            return std::nullopt;
        }

        if(item.count("link") == 0) {
            spdlog::error("Request must contain key 'link'");
            return std::nullopt;
        }

        RequestData to_add;
        to_add.link = item["link"];
                
        if(item.count("required") == 0) {
            to_add.required = false;
        } else {
            to_add.required = item["required"];
        }

        // Convert type field to upper for convenience
        string upper_type(item["type"]);
        std::for_each(upper_type.begin(), upper_type.end(), [](char& c) {c = toupper(c);});

        if(upper_type == "DATA") {
            to_add.type = LinkType::DATA;
        } else if(upper_type == "STREAM") {
            to_add.type = LinkType::STREAM;
        } else {
            spdlog::error("Request type must be STREAM or DATA");
            return std::nullopt;
        }

        ret.push_back(to_add);
    }

    return ret;
}

} // vizier namespace

#endif