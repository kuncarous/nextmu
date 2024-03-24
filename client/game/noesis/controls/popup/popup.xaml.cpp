#include "mu_precompiled.h"
#include "popup.xaml.h"

#include <NsCore/ReflectionImplementEmpty.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/Uri.h>

using namespace NextMU;
using namespace Noesis;

namespace NextMU
{
	Popup::Popup()
	{
		InitializeComponent();
	}

	void Popup::InitializeComponent()
	{
		GUI::LoadComponent(this, "Popup.xaml");
	}
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION_(NextMU::Popup, "NextMU.Popup")

NS_END_COLD_REGION
