#include <utility>
#include <spdlog/spdlog.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <chrono>
#include <thread>

#include "live_driving/obs_client.hpp"

live_driving::obs_client::obs_client(std::string url, std::string password): url(std::move(url)), password(std::move(password)) {
    if(!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    curl = curl_easy_init();
    if(!curl) {
        spdlog::error("Failed to initialize curl");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L);

    connect();
}

void live_driving::obs_client::switch_scene(const std::string& scene_name) const {
    if(!authenticated) {
        return;
    }

    rapidjson::Document payload;
    payload.SetObject();

    rapidjson::Value op_key("op", payload.GetAllocator());
    payload.AddMember(op_key, 6, payload.GetAllocator());

    rapidjson::Value d_key("d", payload.GetAllocator());
    payload.AddMember(d_key, rapidjson::Value(rapidjson::kObjectType), payload.GetAllocator());

    rapidjson::Value request_type_key("requestType", payload.GetAllocator());
    payload["d"].AddMember(request_type_key, "SetCurrentProgramScene", payload.GetAllocator());

    rapidjson::Value request_id_key("requestId", payload.GetAllocator());
    payload["d"].AddMember(request_id_key, "1", payload.GetAllocator());

    rapidjson::Value request_data_key("requestData", payload.GetAllocator());
    payload["d"].AddMember(request_data_key, rapidjson::Value(rapidjson::kObjectType), payload.GetAllocator());

    rapidjson::Value scene_name_key("sceneName", payload.GetAllocator());
    rapidjson::Value scene_name_value(scene_name.c_str(), payload.GetAllocator());
    payload["d"]["requestData"].AddMember(scene_name_key, scene_name_value, payload.GetAllocator());

    send(payload);
}

void live_driving::obs_client::connect() const {
    if(const auto res = curl_easy_perform(curl); res != CURLE_OK) {
        spdlog::error("Failed to connect to websocket server: {}", curl_easy_strerror(res));
    }

    spdlog::info("Connected to websocket server");
}

[[noreturn]] void live_driving::obs_client::listen() {
    size_t return_len;
    const curl_ws_frame* frame;
    constexpr auto allocation_size = 1024;
    char data[allocation_size];

    while(true) {
        const auto result = curl_ws_recv(curl, data, allocation_size, &return_len, &frame);

        if(result == CURLE_OK) {
            if(return_len >= allocation_size) {
                spdlog::error("Received data is too large");
            }

            data[return_len] = '\0';
            spdlog::info("Received data: {}", data);

            handle_response(data);
            continue;
        }

        if(result == CURLE_AGAIN) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        if(result == CURLE_GOT_NOTHING) {
            authenticated = false;
            connect();
            continue;
        }

        spdlog::error("Failed to receive data from websocket server: {} ({})", curl_easy_strerror(result), static_cast<std::underlying_type_t<CURLcode>>(result));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void live_driving::obs_client::handle_response(const char* data) {
    rapidjson::Document document;
    document.Parse(data);

    if(document.HasParseError()) {
        spdlog::error("Failed to parse JSON");
        return;
    }

    if(!document.HasMember("d") || !document.HasMember("op")) {
        spdlog::error("Received data is not a valid OBS message");
        return;
    }

    switch(const auto op = document["op"].GetInt()) {
        case 0:
            handle_authentication(document);
            break;
        case 2:
            authenticated = true;
            spdlog::info("Successfully authenticated with OBS");
            break;
        case 7:
            // Request response, maybe handle this in the future
            break;
        default:
            spdlog::warn("Unsupported operation {}", op);
    }
}

void live_driving::obs_client::handle_authentication(const rapidjson::Document& document) const {
    rapidjson::Document payload;
    payload.SetObject();

    rapidjson::Value op_value("op", payload.GetAllocator());
    payload.AddMember(op_value, 1, payload.GetAllocator());

    rapidjson::Value d_value("d", payload.GetAllocator());
    payload.AddMember(d_value, rapidjson::Value(rapidjson::kObjectType), payload.GetAllocator());

    rapidjson::Value rpc_version_value("rpcVersion", payload.GetAllocator());
    payload["d"].AddMember(rpc_version_value, 1, payload.GetAllocator());

    rapidjson::Value event_subscriptions_value("eventSubscriptions", payload.GetAllocator());
    payload["d"].AddMember(event_subscriptions_value, 33, payload.GetAllocator());

    if(const auto& d = document["d"]; d.HasMember("authentication")) {
        const auto& authentication = d["authentication"];
        const auto challenge = authentication["challenge"].GetString();
        const auto salt = authentication["salt"].GetString();

        std::string base64_secret;

        CryptoPP::SHA256 sha256;
        CryptoPP::StringSource first_crypt(password + salt, true,
            new CryptoPP::HashFilter(sha256,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(base64_secret), false
                )
            )
        );

        std::string authentication_output;

        CryptoPP::StringSource second_crypt(base64_secret + challenge, true,
            new CryptoPP::HashFilter(sha256,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(authentication_output), false
                )
            )
        );

        rapidjson::Value authentication_key("authentication", payload.GetAllocator());
        rapidjson::Value authentication_value(authentication_output.c_str(), payload.GetAllocator());
        payload["d"].AddMember(authentication_key, authentication_value, payload.GetAllocator());
    }

    send(payload);
}

void live_driving::obs_client::send(const rapidjson::Document& document) const {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    document.Accept(writer);
    auto json_string = buffer.GetString();

    spdlog::info("Sending data: {}", json_string);

    size_t sent_len;

    while(true) {
        const auto result = curl_ws_send(curl, json_string, buffer.GetSize(), &sent_len, 0, CURLWS_TEXT);

        if(result == CURLE_OK) {
            break;
        }

        if(result == CURLE_AGAIN) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        spdlog::error("Failed to send data to websocket server: {} ({})", curl_easy_strerror(result), static_cast<std::underlying_type_t<CURLcode>>(result));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
