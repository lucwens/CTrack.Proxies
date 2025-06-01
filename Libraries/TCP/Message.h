#pragma once

// next lines are because of defines in Julia
#undef strtoull
#undef strtoll
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace CTrack
{
    using json = nlohmann::json;
    struct RawJsonTag
    {
    };
    constexpr RawJsonTag raw_json_tag{};

    class Message
    {
      public:
        Message() = default;
        Message(const std::string& id);
        Message(const std::string &id, json params);
        Message(const Message &other);
        Message &operator=(const Message &other);
        Message(Message &&other);
        Message(json raw, RawJsonTag);
        Message           &operator=(Message &&other);
        static Message     Deserialize(const std::string &jsonString);
        const std::string &GetID() const;
        void               SetID(const std::string_view &id);
        const json        &GetParams() const;
        json              &GetParams();
        void               SetParams(const json &params);
        const json        &Raw() const;
        std::string        Serialize() const;
        void               DebugUpdate();

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
