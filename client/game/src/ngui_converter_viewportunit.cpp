#include "mu_precompiled.h"
#include "ngui_converter_viewportunit.h"
#include "mu_graphics.h"

bool ViewportWidthConverter::TryConvert(Noesis::BaseComponent */*value*/, const Noesis::Type *type, Noesis::BaseComponent *parameter,
	Noesis::Ptr<Noesis::BaseComponent> &result)
{
	if (parameter == nullptr || type != Noesis::TypeOf<float>()) return false;
	mu_boolean unboxed = false;
	mu_float value = 0.0f;
	if (Noesis::Boxing::CanUnbox<mu_int32>(parameter))
	{
		unboxed = true;
		value = static_cast<mu_float>(Noesis::Boxing::Unbox<mu_int32>(parameter));
	}
	else if (Noesis::Boxing::CanUnbox<mu_float>(parameter))
	{
		unboxed = true;
		value = Noesis::Boxing::Unbox<mu_float>(parameter);
	}
	else if (Noesis::Boxing::CanUnbox<mu_double>(parameter))
	{
		unboxed = true;
		value = static_cast<mu_float>(Noesis::Boxing::Unbox<mu_double>(parameter));
	}
	else if (Noesis::Boxing::CanUnbox<Noesis::String>(parameter))
	{
		unboxed = true;
		const mu_utf8string str = Noesis::Boxing::Unbox<Noesis::String>(parameter).Str();
		value = static_cast<mu_float>(std::stof(str));
		if (std::isinf(value) || std::isnan(value)) return false;
	}
	if (unboxed == false) return false;
	const auto &swapchainDesc = MUGraphics::GetSwapChain()->GetDesc();
	result = Noesis::Boxing::Box(value * 0.01f * static_cast<mu_float>(glm::max(swapchainDesc.Width, swapchainDesc.Height)));
	return true;
}

bool ViewportHeightConverter::TryConvert(Noesis::BaseComponent */*value*/, const Noesis::Type *type, Noesis::BaseComponent *parameter,
	Noesis::Ptr<Noesis::BaseComponent> &result)
{
	if (parameter == nullptr || type != Noesis::TypeOf<float>()) return false;
	mu_boolean unboxed = false;
	mu_float value = 0.0f;
	if (Noesis::Boxing::CanUnbox<mu_int32>(parameter))
	{
		unboxed = true;
		value = static_cast<mu_float>(Noesis::Boxing::Unbox<mu_int32>(parameter));
	}
	else if (Noesis::Boxing::CanUnbox<mu_float>(parameter))
	{
		unboxed = true;
		value = Noesis::Boxing::Unbox<mu_float>(parameter);
	}
	else if (Noesis::Boxing::CanUnbox<mu_double>(parameter))
	{
		unboxed = true;
		value = static_cast<mu_float>(Noesis::Boxing::Unbox<mu_double>(parameter));
	}
	else if (Noesis::Boxing::CanUnbox<Noesis::String>(parameter))
	{
		unboxed = true;
		const mu_utf8string str = Noesis::Boxing::Unbox<Noesis::String>(parameter).Str();
		value = static_cast<mu_float>(std::stof(str));
		if (std::isinf(value) || std::isnan(value)) return false;
	}
	if (unboxed == false) return false;
	const auto &swapchainDesc = MUGraphics::GetSwapChain()->GetDesc();
	result = Noesis::Boxing::Box(value * 0.01f * static_cast<mu_float>(glm::min(swapchainDesc.Height, swapchainDesc.Width)));
	return true;
}