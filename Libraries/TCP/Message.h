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

    class Message
    {
      public:
        Message() = default;

        Message(std::string id)
        {
            data_["id"]     = std::move(id);
            DebugUpdate();
        }
        Message(std::string id, json params)
        {
            data_["id"]     = std::move(id);
            data_["params"] = std::move(params);
            DebugUpdate();
        }

        // Allow copy constructor and copy assignment operator
        Message(const Message &other)
        {
            data_    = other.data_;
#ifdef _DEBUG
            debugMessage_ = other.debugMessage_;
#endif
        }
        Message &operator=(const Message &other)
        {
            if (this != &other)
            { // Check for self-assignment
                data_ = other.data_;
#ifdef _DEBUG
                debugMessage_ = other.debugMessage_;
#endif
                DebugUpdate();
            }
            return *this; // Ensure the function returns *this
        };

        // Allow move constructor and move assignment operator
        Message(Message &&other)
        {
            if (this != &other)
            { // Check for self-assignment
                data_ = std::move(other.data_);
#ifdef _DEBUG
                debugMessage_ = std::move(other.debugMessage_);
#endif
            }
        };
        Message &operator=(Message &&other)
        {
            if (this != &other)
            { // Check for self-assignment
                data_ = std::move(other.data_);
#ifdef _DEBUG
                debugMessage_ = std::move(other.debugMessage_);
#endif
            }
            return *this; // Ensure the function returns *this
        };

//        explicit Message(json raw) : data_(std::move(raw)) { DebugUpdate(); }

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
        void               SetID(const std::string_view &id)
        {
            data_["id"] = id;
            DebugUpdate();
        }
        const json &GetParams() const { return data_.at("params"); }
        json       &GetParams() { return data_["params"]; }
        void        SetParams(const json &params)
        {
            data_["params"] = params;
            DebugUpdate();
        }
        const json &Raw() const { return data_; }
        std::string Serialize() const { return data_.dump(); }

        void DebugUpdate()
        {
#ifdef _DEBUG
            debugMessage_ = Serialize();
#endif
        };

      private:
        json data_;
#ifdef _DEBUG
        std::string debugMessage_; // For debugging purposes, can be used to store a message about the content of the message
#endif
    };
    
    using Reply     = std::unique_ptr<Message>;
    using Handler   = std::function<Reply(const Message &)>;
    using HandlerID = size_t;
} // namespace CTrack
