#ifndef __T_JOINT_MACROS_H__
#define __T_JOINT_MACROS_H__

#pragma once

#define REGISTER_INVOKE(func) \
		invokes.##func.insert(std::make_pair(Type, func));

#endif