#include "mu_precompiled.h"
#include "popups.xaml.h"

#include <NsCore/ReflectionImplementEmpty.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/Uri.h>

using namespace NextMU;
using namespace Noesis;

namespace NextMU
{
	Popups::Popups()
	{
		InitializeComponent();
	}

	void Popups::InitializeComponent()
	{
		GUI::LoadComponent(this, "Popups.xaml");
	}
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION_(NextMU::Popups, "NextMU.Popups")

NS_END_COLD_REGION
