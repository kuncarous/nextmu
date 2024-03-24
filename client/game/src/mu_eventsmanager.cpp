#include "mu_precompiled.h"
#include "mu_eventsmanager.h"

namespace MUEventsManager
{
    std::mutex Mutex;
    std::vector<NEventRequestPtr> Events;

    void Process()
    {
        std::lock_guard lock(Mutex);
        for (auto iter = Events.begin(); iter != Events.end(); ++iter)
        {
            NEventRequestPtr &event = *iter;
            event->Process();
        }
        Events.clear();
    }

    void AddEvent(NEventRequestPtr event)
    {
        std::lock_guard lock(Mutex);
        Events.push_back(std::move(event));
    }
};