#include "Subscription.h"
#include "MessageResponder.h"

namespace CTrack
{
    Subscription::~Subscription()
    {
        Unsubscribe();
    }

    void Subscription::Unsubscribe()
    {
        if (auto r = router_.lock())
        {
            r->Unsubscribe(messageID_, handlerID_);
        }
    }
} // namespace CTrack
