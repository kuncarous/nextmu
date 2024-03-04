#include "mu_precompiled.h"
#include "ngui_context.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
NGUpdateContext *NGApplicationContext::GetUpdate() const
{
	return Update;
}

void NGApplicationContext::SetUpdate(NGUpdateContext *value)
{
    if (Update == value) return;
    Update.Reset(value);
    OnPropertyChanged("Update");
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(NGApplicationContext, "ApplicationContext")
{
    NsImpl<Noesis::INotifyPropertyChanged>();
    NsProp("Update", &NGApplicationContext::GetUpdate, &NGApplicationContext::SetUpdate);
}

NS_END_COLD_REGION
#endif