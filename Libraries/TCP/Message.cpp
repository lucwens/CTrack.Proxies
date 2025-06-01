#include "Message.h"

namespace CTrack
{
    // Constructor with id and params
    Message::Message(const std::string& id)
    {
        data_["id"]     = id;
        data_["params"] = {};
        DebugUpdate();
    }

    // Constructor with id and params
    Message::Message(const std::string &id, json params)
    {
        data_["id"]     = id;
        data_["params"] = std::move(params);
        DebugUpdate();
    }

    // Copy constructor
    Message::Message(const Message &other)
    {
        data_ = other.data_;
#ifdef _DEBUG
        debugMessage_ = other.debugMessage_;
#endif
    }

    // Copy assignment
    Message &Message::operator=(const Message &other)
    {
        if (this != &other)
        {
            data_ = other.data_;
#ifdef _DEBUG
            debugMessage_ = other.debugMessage_;
#endif
            DebugUpdate();
        }
        return *this;
    }

    // Move constructor
    Message::Message(Message &&other)
    {
        if (this != &other)
        {
            data_ = std::move(other.data_);
#ifdef _DEBUG
            debugMessage_ = std::move(other.debugMessage_);
#endif
        }
    }

    // Move assignment
    Message &Message::operator=(Message &&other)
    {
        if (this != &other)
        {
            data_ = std::move(other.data_);
#ifdef _DEBUG
            debugMessage_ = std::move(other.debugMessage_);
#endif
        }
        return *this;
    }

    // Static: Deserialize from string
    Message Message::Deserialize(const std::string &jsonString)
    {
        json parsed = json::parse(jsonString);
        if (!parsed.contains("id") || !parsed["id"].is_string())
        {
            throw std::invalid_argument("JSON missing 'id' string");
        }
        if (!parsed.contains("params"))
        {
            parsed["params"] = {{}};
        }
        return Message(std::move(parsed), raw_json_tag);
    }

    // GetID
    const std::string &Message::GetID() const
    {
        return data_.at("id").get_ref<const std::string &>();
    }

    // SetID
    void Message::SetID(const std::string_view &id)
    {
        data_["id"] = id;
        DebugUpdate();
    }

    // GetParams (const)
    const json &Message::GetParams() const
    {
        return data_.at("params");
    }

    // GetParams (non-const)
    json &Message::GetParams()
    {
        return data_["params"];
    }

    // SetParams
    void Message::SetParams(const json &params)
    {
        data_["params"] = params;
        DebugUpdate();
    }

    // Raw
    const json &Message::Raw() const
    {
        return data_;
    }

    // Serialize
    std::string Message::Serialize() const
    {
        return data_.dump();
    }

    // DebugUpdate
    void Message::DebugUpdate()
    {
#ifdef _DEBUG
        debugMessage_ = Serialize();
#endif
    }

    // Private constructor from json (not in header, but needed for Deserialize)
    Message::Message(json raw, RawJsonTag) : data_(std::move(raw))
    {
        DebugUpdate();
    }
} // namespace CTrack
