#ifndef __T_GRAPHICS_BUFFER_H__
#define __T_GRAPHICS_BUFFER_H__

#pragma once

#include "t_graphics_resources.h"

class NGraphicsBuffer : public NGraphicsResource
{
public:
	NGraphicsBuffer();

	const mu_boolean Create(const Diligent::BufferDesc &bufferDesc, const Diligent::BufferData *bufferData = nullptr);
	void Destroy();

public:
	const mu_boolean IsValid() const
	{
		return Buffer != nullptr;
	}

	Diligent::IBuffer *GetBuffer()
	{
		return Buffer.RawPtr();
	}

protected:
	Diligent::RefCntAutoPtr<Diligent::IBuffer> Buffer;
};

#endif