#ifndef __UPD_ENUM_H__
#define __UPD_ENUM_H__

#pragma once

enum class NUpdateState : mu_uint32
{
    Initialize,
    WaitingServers,
    WaitingFilesList,
    VerifyingFiles,
    UpdatingFiles,
    WritingVersion,
    Finished,
};

#endif