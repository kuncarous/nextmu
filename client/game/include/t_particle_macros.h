#ifndef __T_PARTICLE_MACROS_H__
#define __T_PARTICLE_MACROS_H__

#pragma once

#define REGISTER_INVOKE(func) \
		invokes.##func.insert(std::make_pair(Type, func));

#endif