#pragma once

#include "Message.h"
#include "MessageResponder.h"
#include "Subscription.h"
#include <vector>
#include <memory>

namespace CTrack
{
    template <typename T> auto MakeMemberHandler(T *obj, std::unique_ptr<Message> (T::*fn)(const Message &))
    {
        return [obj, fn](const Message &msg) { return (obj->*fn)(msg); };
    }

    class Subscriber
    {
      public:
        Subscriber()          = default;
        virtual ~Subscriber() { ClearSubscriptions(); };

        void Subscribe(MessageResponder &messageResponder, const std::string &messageID, Handler handler)
        {
            TrackSubscription(messageResponder.Subscribe(messageID, handler));
        };

      protected:
        // Call this to add a subscription; ownership kept here.
        void TrackSubscription(Subscription &&sub) { subscriptions_.emplace_back(std::move(sub)); }
        void ClearSubscriptions() { subscriptions_.clear(); }

      private:
        std::vector<Subscription> subscriptions_;
    };

} // namespace CTrack
