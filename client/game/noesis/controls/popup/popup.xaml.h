#ifndef __NEXTMU_POPUP_H__
#define __NEXTMU_POPUP_H__

#include <NsCore/Noesis.h>
#include <NsGui/UserControl.h>

namespace NextMU
{
    class Popup final: public Noesis::UserControl
    {
    public:
        Popup();

    private:
        void InitializeComponent();

        NS_DECLARE_REFLECTION(Popup, UserControl)
    };
}


#endif
