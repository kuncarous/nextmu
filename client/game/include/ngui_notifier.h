#ifndef __NGUI_NOTIFIER_H__
#define __NGUI_NOTIFIER_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include <NsCore/Noesis.h>
#include <NsCore/ReflectionImplement.h>
#include <NsGui/INotifyPropertyChanged.h>

namespace NoesisGUI
{
    class NotifierBase: public Noesis::BaseComponent, public Noesis::INotifyPropertyChanged
    {
    public:
        Noesis::PropertyChangedEventHandler& PropertyChanged() final;

        NS_IMPLEMENT_INTERFACE_FIXUP

    protected:
        void OnPropertyChanged(const char* propertyName);

    private:
        Noesis::PropertyChangedEventHandler _changed;

        NS_DECLARE_REFLECTION(NotifierBase, BaseComponent)
    };
};
#endif

#endif