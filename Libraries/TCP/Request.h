#pragma once

#include "Message.h"
#include "Subscription.h"

#include <future>
#include <memory>

namespace CTrack
{
    class Message;
    class MessageResponder;
    using Reply   = std::unique_ptr<Message>;
    using Handler = std::function<Reply(const Message &)>;

    class Request
    {
      public:
        Request(MessageResponder &, const Message &, Handler = {});
        // Delete copy constructor and copy assignment operator
        Request(const Request &)            = delete;
        Request &operator=(const Request &) = delete;

        // Allow move constructor and move assignment operator
        Request(Request &&)                 = default;
        Request &operator=(Request &&)      = default;

        void                 SetHandler(Handler handler);
        std::future<Message> GetReplyFuture();
        void                 SetReply(const Message &);

      private:
        std::promise<Message> replyPromise;
        Handler               handler; // optional extra function that gets called on the reply
    };
} // namespace CTrack
