#ifndef __NEXTMU_POPUPS_H__
#define __NEXTMU_POPUPS_H__

#include <NsCore/Noesis.h>
#include <NsGui/UserControl.h>

namespace NextMU
{
    class Popups final: public Noesis::UserControl
    {
    public:
        Popups();

    private:
        void InitializeComponent();

        NS_DECLARE_REFLECTION(Popups, UserControl)
    };
}


#endif
