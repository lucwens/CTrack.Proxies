
#include "Subscription.h"
#include "Request.h"

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CTrack
{
    class MessageResponder : public std::enable_shared_from_this<MessageResponder>
    {
      public:
        void                       SetSendFunction(std::function<void(const std::string &)>);
        [[nodiscard]] Subscription Subscribe(const std::string &id, Handler);
        void                       ReceiveMessage(const Message &);
        void                       SendMessage(const std::string &id, const json &);
        void                       SendMessage(const Message &);
        std::future<Message>       SendRequest(const Message &, Handler = {});
        void                       Unsubscribe(const std::string &id, HandlerID handlerID);

      private:
        mutable std::mutex                                                      mutex_;
        std::unordered_map<std::string, std::unordered_map<HandlerID, Handler>> handlers_;
        HandlerID                                                               nextHandlerID_ = 0;
        std::function<void(const std::string &)>                                sendFunction_;

      private:
        mutable std::mutex                       requestsMutex_;
        std::unordered_map<std::string, Request> requests_;
    };
} // namespace CTrack
