#include "Request.h"
#include "MessageResponder.h"

namespace CTrack
{
    Request::Request(MessageResponder &messageRouter, const Message &message, Handler i_handler)
    {
        handler = std::move(i_handler);
    }

    void Request::SetReply(const Message &message)
    {
        replyPromise.set_value(message);
        if (handler)
        {
            handler(message);
        }
    }

    void Request::SetHandler(Handler handler)
    {
        this->handler = std::move(handler);
    }

    std::future<Message> Request::GetReplyFuture()
    {
        return replyPromise.get_future();
    }
} // namespace CTrack
