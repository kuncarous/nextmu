#ifndef __N_UIEVENT_UPDATECONSOLE_H__
#define __N_UIEVENT_UPDATECONSOLE_H__

#pragma once

#include "n_uievent_base.h"

class UIUpdateConsoleEvent : public NCustomEvent
{
public:
    UIUpdateConsoleEvent() : NCustomEvent(NEventType::UpdateConsole) {}
};

#endif
