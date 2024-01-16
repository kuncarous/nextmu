#include "mu_precompiled.h"
#include "t_graphics_buffer.h"
#include "mu_graphics.h"

NGraphicsBuffer::NGraphicsBuffer() : NGraphicsResource(NGraphicsResourceType::Buffer)
{

}

const mu_boolean NGraphicsBuffer::Create(const Diligent::BufferDesc &bufferDesc, const Diligent::BufferData *bufferData)
{
	const auto device = MUGraphics::GetDevice();
	device->CreateBuffer(bufferDesc, bufferData, &Buffer);
	if (Buffer == nullptr)
	{
		return false;
	}

	return true;
}

void NGraphicsBuffer::Destroy()
{
	Buffer.Release();
}