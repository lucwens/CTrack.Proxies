#include "MessageResponder.h"

namespace CTrack
{
    using Reply     = std::unique_ptr<Message>;
    using Handler   = std::function<Reply(const Message &)>;
    using HandlerID = size_t;

    void MessageResponder::SetSendFunction(std::function<void(const std::string &)> sendFunction)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sendFunction_ = std::move(sendFunction);
    }

    [[nodiscard]]
    Subscription MessageResponder::Subscribe(const std::string &id, Handler handler)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        HandlerID                   newID = nextHandlerID_++;
        handlers_[id][newID]              = std::move(handler);
        return Subscription(this->shared_from_this(), id, newID);
    }

    void MessageResponder::RespondToMessage(const Message &message)
    {
        // standard handlers
        std::vector<Handler> copiedHandlers;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            const std::string          &id = message.GetID();
            auto                        it = handlers_.find(id);
            if (it != handlers_.end())
            {
                for (auto &pair : it->second)
                {
                    copiedHandlers.push_back(pair.second);
                }
            }
        }
        // Call handlers outside lock for deadlock safety
        for (auto &handler : copiedHandlers)
        {
            if (auto reply = handler(message))
            {
                SendMessage(*reply);
            }
        }

        // request handlers
        std::lock_guard<std::mutex> lock(requestsMutex_);
        auto                        it = requests_.find(message.GetID());
        if (it != requests_.end())
        {
            auto &request = it->second;
            request.SetReply(message);
            requests_.erase(it);
        }
    }

    void MessageResponder::SendMessage(const std::string &id, const json &params)
    {
        Message message(id, params);
        SendMessage(message);
    }

    void MessageResponder::SendMessage(const Message &message)
    {
        std::function<void(const std::string &)> sendCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sendCopy = sendFunction_;
        }
        if (!sendCopy)
        {
            std::cerr << "No send callback set.\n";
            return;
        }
        sendCopy(message.Serialize());
    }

    std::future<Message> MessageResponder::SendRequest(const Message &message, Handler handler)
    {
        std::lock_guard<std::mutex> lock(requestsMutex_);
        requests_.emplace(message.GetID(), Request(*this, message, handler));
        auto iter = requests_.find(message.GetID());
        if (iter == requests_.end())
        {
            throw std::runtime_error("Failed to create request for message ID: " + message.GetID());
        }

        SendMessage(message);
        return iter->second.GetReplyFuture();
    }

    void MessageResponder::Unsubscribe(const std::string &id, HandlerID handlerID)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto                        it = handlers_.find(id);
        if (it != handlers_.end())
        {
            it->second.erase(handlerID);
            if (it->second.empty())
            {
                handlers_.erase(it);
            }
        }
    }
} // namespace CTrack
