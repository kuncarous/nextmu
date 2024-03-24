#ifndef __NGUI_CONVERTER_VIEWPORTUNIT_H__
#define __NGUI_CONVERTER_VIEWPORTUNIT_H__

#pragma once

#include <NsGui/BaseValueConverter.h>

class ViewportWidthConverter : public Noesis::BaseValueConverter
{
public:
	bool TryConvert(Noesis::BaseComponent *value, const Noesis::Type *type, Noesis::BaseComponent * /*parameter*/,
		Noesis::Ptr<Noesis::BaseComponent> &result) override;

	NS_IMPLEMENT_INLINE_REFLECTION_(ViewportWidthConverter, Noesis::BaseValueConverter, "NextMU.ViewportWidthConverter")
};

class ViewportHeightConverter: public Noesis::BaseValueConverter
{
public:
    bool TryConvert(Noesis::BaseComponent* value, const Noesis::Type* type, Noesis::BaseComponent* /*parameter*/,
        Noesis::Ptr<Noesis::BaseComponent>& result) override;

    NS_IMPLEMENT_INLINE_REFLECTION_(ViewportHeightConverter, Noesis::BaseValueConverter, "NextMU.ViewportHeightConverter")
};

#endif