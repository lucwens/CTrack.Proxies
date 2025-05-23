#pragma once

#include <memory>
#include <string>
#include <utility>

namespace CTrack
{
    class MessageResponder; // Forward declaration

    class [[nodiscard]] Subscription
    {
        using HandlerID = size_t;

      public:
        Subscription(std::weak_ptr<MessageResponder> router, std::string id, HandlerID handlerID)
            : router_(std::move(router)), messageID_(std::move(id)), handlerID_(handlerID)
        {
        }
        ~Subscription();
        Subscription()                                = default;
        // Enable move/copy semantics for use in containers
        Subscription(const Subscription &)            = default;
        Subscription(Subscription &&)                 = default;
        Subscription &operator=(const Subscription &) = default;
        Subscription &operator=(Subscription &&)      = default;

        void Unsubscribe();

      private:
        std::weak_ptr<MessageResponder> router_;
        std::string           messageID_;
        HandlerID             handlerID_ = 0;
    };
} // namespace CTrack
