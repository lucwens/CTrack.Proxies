#include "Request.h"
#include "MessageResponder.h"

namespace CTrack
{
    Request::Request(const Message &message, Handler i_handler)
    {
        handler = std::move(i_handler);
    }

    Reply Request::SetReply(const Message &message)
    {
        Reply reply;
        replyPromise.set_value(message);
        if (handler)
        {
            reply = handler(message);
        }
        return reply;
    }

    void Request::SetHandler(Handler handler)
    {
        this->handler = std::move(handler);
    }

    Handler &&Request::TakeHandler()
    {
        return std::move(handler);
    }

    std::future<Message> Request::GetReplyFuture()
    {
        return replyPromise.get_future();
    }
} // namespace CTrack
