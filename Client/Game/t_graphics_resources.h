#ifndef __T_GRAPHICS_RESOURCES_H__
#define __T_GRAPHICS_RESOURCES_H__

#pragma once

enum class NGraphicsResourceType : mu_uint32
{
	Texture,
	Buffer,
};

typedef mu_uint32 NResourceId;
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

protected:
	friend mu_uint32 GenerateResourceId();
	static mu_atomic_uint32_t IdGenerator;
};

mu_uint32 GenerateResourceId();

#endif