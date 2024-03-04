#include "mu_precompiled.h"
#include "ngui_notifier.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace NoesisGUI
{
    Noesis::PropertyChangedEventHandler& NotifierBase::PropertyChanged()
    {
        return _changed;
    }

    void NotifierBase::OnPropertyChanged(const char* propertyName)
    {
        if (!_changed.Empty())
        {
            Noesis::Symbol propId(propertyName);
            _changed(this, Noesis::PropertyChangedEventArgs(propId));
        }
    }

	NS_BEGIN_COLD_REGION

	NS_IMPLEMENT_REFLECTION(NotifierBase)
	{
		NsImpl<INotifyPropertyChanged>();
	}

	NS_END_COLD_REGION
};
#endif