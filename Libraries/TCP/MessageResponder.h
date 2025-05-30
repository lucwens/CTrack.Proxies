#pragma once
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
#include <deque>

namespace CTrack
{
    class MessageResponder : public std::enable_shared_from_this<MessageResponder>
    {
      public:
        void                       SetSendFunction(std::function<void(Message &)>);
        [[nodiscard]] Subscription Subscribe(const std::string &id, Handler);
        void                       RespondToMessage(const Message &);
        void                       SendMessage(const std::string &id, const json &);
        void                       SendMessage(Message &);
        void                       SendRequest(Message &, Handler);
        void                       SendRequest(Message &, std::future<Message> &);
        void                       Unsubscribe(const std::string &id, HandlerID handlerID);
        void                       RequestSetPromiseThread(const Message &message);

      private:
        mutable std::mutex                                                      mutex_;
        std::unordered_map<std::string, std::unordered_map<HandlerID, Handler>> handlers_;
        HandlerID                                                               nextHandlerID_ = 0;
        std::function<void(Message &)>                                          sendFunction_;

      private:
        mutable std::recursive_mutex             requestsMutex_;
        std::unordered_map<std::string, Request> requests_;
    };

    // functions to handles a list of requests
    struct RequestItem
    {
        Message message;
        Handler handler;
        RequestItem(Message msg, Handler hndlr) : message(std::move(msg)), handler(std::move(hndlr)) {}
    };

    void RequestList(MessageResponder &, std::deque<RequestItem> &list);

} // namespace CTrack
