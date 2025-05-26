#pragma once

// next lines are because of defines in Julia 
#undef strtoull
#undef strtoll
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace CTrack
{
    using json      = nlohmann::json;
    using HandlerID = size_t;

    class Message
    {
      public:
        Message() = default;

        Message(std::string id, json params)
        {
            data_["id"]     = std::move(id);
            data_["params"] = std::move(params);
        }

        // Allow copy constructor and copy assignment operator
        Message(const Message &)            = default;
        Message &operator=(const Message &) = default;

        // Allow move constructor and move assignment operator
        Message(Message &&)                 = default;
        Message &operator=(Message &&)      = default;

        explicit Message(json raw) : data_(std::move(raw)) {}

        static Message Deserialize(const std::string &jsonString)
        {
            json parsed = json::parse(jsonString);
            if (!parsed.contains("id") || !parsed["id"].is_string())
            {
                throw std::invalid_argument("JSON missing 'id' string");
            }
            if (!parsed.contains("params"))
            {
                throw std::invalid_argument("JSON missing 'params'");
            }
            return Message(std::move(parsed));
        }
        // GetID and GetParams will throw if the keys are not present or not of the expected type
        const std::string &GetID() const { return data_.at("id").get_ref<const std::string &>(); }
        void               SetID(const std::string_view &id) { data_["id"] = id; }
        const json        &GetParams() const { return data_.at("params"); }
        json              &GetParams() { return data_["params"]; }
        void               SetParams(const json &params) { data_["params"] = params; }
        const json        &Raw() const { return data_; }
        std::string        Serialize() const { return data_.dump(); }

      private:
        json data_;
    };
} // namespace CTrack
