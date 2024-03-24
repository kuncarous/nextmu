#include "mu_precompiled.h"
#include "fixedwindow.xaml.h"

#include <NsCore/ReflectionImplementEmpty.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/Uri.h>

using namespace NextMU;
using namespace Noesis;

namespace NextMU
{
	FixedWindow::FixedWindow()
	{
		InitializeComponent();
	}

	void FixedWindow::InitializeComponent()
	{
		GUI::LoadComponent(this, "FixedWindow.xaml");
	}
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION_(NextMU::FixedWindow, "NextMU.FixedWindow")

NS_END_COLD_REGION
