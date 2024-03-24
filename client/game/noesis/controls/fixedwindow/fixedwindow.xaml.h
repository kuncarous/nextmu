#ifndef __NEXTMU_FIXEDWINDOW_H__
#define __NEXTMU_FIXEDWINDOW_H__

#include <NsCore/Noesis.h>
#include <NsGui/UserControl.h>

namespace NextMU
{
    class FixedWindow final: public Noesis::UserControl
    {
    public:
        FixedWindow();

    private:
        void InitializeComponent();

        NS_DECLARE_REFLECTION(FixedWindow, UserControl)
    };
}


#endif
