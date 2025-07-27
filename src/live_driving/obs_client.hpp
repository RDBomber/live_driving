#pragma once

#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>

#include "live_driving/config.hpp"

namespace live_driving {
    class obs_client {
    public:
        obs_client(std::string url, std::string password);

        void listen();
        void switch_scene(const scene_config& config) const;

    private:
        static inline bool curl_initialized = false;

        bool authenticated = false;
        bool connected = false;

        std::string url;
        std::string password;

        CURL* curl = nullptr;

        void connect();

        void handle_response(const char* data);
        void handle_authentication(const rapidjson::Document& document) const;
        void send(const rapidjson::Document& document) const;
    };
}
