#ifndef __MU_EVENTSMANAGER_H__
#define __MU_EVENTSMANAGER_H__

#pragma once

class NEventRequest;
typedef std::unique_ptr<NEventRequest> NEventRequestPtr;

namespace MUEventsManager
{
    void Process();
    void AddEvent(NEventRequestPtr event);
};

class NEventRequest
{
public:
	virtual ~NEventRequest() {}

private:
	friend void MUEventsManager::Process();
	virtual void Process() {}
};

#endif