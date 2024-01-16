#ifndef __SHARED_OPERATINGSYSTEM_DISABLE_H__
#define __SHARED_OPERATINGSYSTEM_DISABLE_H__

#pragma once

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
// Disable annoying Windows features
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOATOM
//#define NOGDI
#define NOKERNEL
#define NOMEMMGR
#define NOSERVICE
#define NOWH
#define NOCOMM
#define NOPROFILER
#define NOMCX

#if !defined(NEXTMU_SERVER)
#define NODRAWTEXT
#define NOTEXTMETRIC
#define NOHELP
#define NOMINMAX
#endif
#endif

#endif