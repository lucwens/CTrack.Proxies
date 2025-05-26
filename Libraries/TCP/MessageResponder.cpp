#include "MessageResponder.h"

namespace CTrack
{
    using Reply     = std::unique_ptr<Message>;
    using Handler   = std::function<Reply(const Message &)>;
    using HandlerID = size_t;

    void MessageResponder::SetSendFunction(std::function<void(Message &)> sendFunction)
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
        std::lock_guard<std::recursive_mutex> lock(requestsMutex_);
        auto                        it = requests_.find(message.GetID());
        if (it != requests_.end())
        {
            auto &request = it->second;
            auto  reply   = request.SetReply(message);
            if (reply)
            {
                Handler handler = std::move(request.TakeHandler());
                if (handler)
                {
                    SendRequest(*reply, handler);
                };
            }
            requests_.erase(it);
        }
    }

    void MessageResponder::SendMessage(const std::string &id, const json &params)
    {
        Message message(id, params);
        SendMessage(message);
    }

    void MessageResponder::SendMessage(Message &message)
    {
        std::function<void(Message &)> sendCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sendCopy = sendFunction_;
        }
        if (!sendCopy)
        {
            std::cerr << "No send callback set.\n";
            return;
        }
        sendCopy(message);
    }

    void MessageResponder::SendRequest(Message &message, Handler handler)
    {
        std::lock_guard<std::recursive_mutex> lock(requestsMutex_);
        auto [iter, inserted] = requests_.emplace(message.GetID(), Request(message, handler));
        if (!inserted)
        {
            throw std::runtime_error("Failed to create request for message ID: " + message.GetID());
        }

        SendMessage(message);
    }

    void MessageResponder::SendRequest(Message &message, std::future<Message> &future)
    {
        std::lock_guard<std::recursive_mutex> lock(requestsMutex_);
        auto [iter, inserted] = requests_.emplace(message.GetID(), Request(message, {}));
        if (!inserted)
        {
            throw std::runtime_error("Failed to create request for message ID: " + message.GetID());
        }
        future = std::move(iter->second.GetReplyFuture());
        SendMessage(message);
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

    // list of requests


    void NextRequest(MessageResponder &responder, std::shared_ptr<std::deque<RequestItem>> queue)
    {
        if (queue->empty())
        {
            return;
        }
        RequestItem item = std::move(queue->front());
        queue->pop_front();

        // Recursive callback
        Handler cb = [queue, item, &responder](const Message &reply) -> Reply
        {
            if (item.handler)
                item.handler(reply);       // call the handler for this item
            NextRequest(responder, queue); // only now send the next
            return nullptr;                // no reply expected
        };

        responder.SendRequest(item.message, cb);
    }

    void RequestList(MessageResponder &messageResponder, std::deque<RequestItem> &list)
    {
        auto queue = std::make_shared<std::deque<RequestItem>>(std::move(list));
        NextRequest(messageResponder, queue);
    }

} // namespace CTrack
