#ifndef __N_UIEVENT_UPDATEFPS_H__
#define __N_UIEVENT_UPDATEFPS_H__

#pragma once

#include "n_uievent_base.h"

class UIUpdateFPSEvent : public NCustomEvent
{
public:
    UIUpdateFPSEvent(
        const mu_uint32 fps,
        const mu_double workTime,
        const mu_double elapsedTime
    ) :
        NCustomEvent(NEventType::UpdateFPS),
        FPS(fps),
        WorkTime(workTime),
        ElapsedTime(elapsedTime)
    {}

public:
    const mu_uint32 FPS;
    const mu_double WorkTime;
    const mu_double ElapsedTime;
};

#endif
