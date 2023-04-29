#ifndef __T_GRAPHICS_RESOURCES_H__
#define __T_GRAPHICS_RESOURCES_H__

#pragma once

#include "t_graphics_resourceid.h"

enum class NGraphicsResourceType : mu_uint32
{
	Texture,
	Buffer,
};

class NGraphicsResource
{
public:
	NGraphicsResource(const NGraphicsResourceType type);
	virtual ~NGraphicsResource();

public:
	const NGraphicsResourceType GetType() const
	{
		return Type;
	}

	const mu_uint32 GetId() const
	{
		return Id;
	}

protected:
	NGraphicsResourceType Type;
	NResourceId Id;
};

#endif