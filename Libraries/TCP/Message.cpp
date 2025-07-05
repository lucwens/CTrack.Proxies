#include "Message.h"

constexpr const char *ParamsKey = "params";
constexpr const char *IDKey     = "id";

namespace CTrack
{
    // Constructor with id and params
    Message::Message(const std::string &id)
    {
        data_[IDKey]     = id;
        data_[ParamsKey] = {};
        DebugUpdate();
    }

    // Constructor with id and params
    Message::Message(const std::string &id, json params)
    {
        data_[IDKey]     = id;
        data_[ParamsKey] = std::move(params);
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
        if (!parsed.contains(IDKey) || !parsed[IDKey].is_string())
        {
            throw std::invalid_argument("JSON missing 'id' string");
        }
        if (!parsed.contains(ParamsKey))
        {
            parsed[ParamsKey] = {{}};
        }
        return Message(std::move(parsed), raw_json_tag);
    }

    // GetID
    const std::string &Message::GetID() const
    {
        return data_.at(IDKey).get_ref<const std::string &>();
    }

    // SetID
    void Message::SetID(const std::string_view &id)
    {
        data_[IDKey] = id;
        DebugUpdate();
    }

    const bool Message::HasParams() const
    {
        if (!data_.contains(ParamsKey))
            return false;
        if (GetParams() == nullptr)
            return false;
        return true;
    }

    // GetParams (const)
    const json &Message::GetParams() const
    {
        if (data_.find(ParamsKey) == data_.end())
        {
            data_[ParamsKey] = json::object();
        }
        if (data_[ParamsKey] == nullptr)
        {
            data_[ParamsKey] = json::object();
        }

        return data_.at(ParamsKey);
    }

    // GetParams (non-const)
    json &Message::GetParams()
    {
        if (data_.find(ParamsKey) == data_.end())
        {
            // If the key does not exist, add it with an empty JSON object
            data_[ParamsKey] = json::object();
        }
        return data_[ParamsKey];
    }

    // SetParams
    void Message::SetParams(const json &params)
    {
        data_[ParamsKey] = params;
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
