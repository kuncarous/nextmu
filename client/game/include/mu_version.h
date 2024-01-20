#ifndef __MU_VERSION_H__
#define __MU_VERSION_H__

#pragma once

#define GAME_VERSION_PARSER(major, minor, revision) #major "." #minor "." #revision
#define GAME_VERSION			GAME_VERSION_PARSER(NEXTMU_VERSION_MAJOR, NEXTMU_VERSION_MINOR, NEXTMU_VERSION_REVISION)

#endif