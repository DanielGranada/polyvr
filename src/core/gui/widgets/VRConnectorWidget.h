#ifndef VRCONNECTORWIDGET_H_INCLUDED
#define VRCONNECTORWIDGET_H_INCLUDED

#include "core/gui/VRGuiFwd.h"
#include "addons/Semantics/VRSemanticsFwd.h"

#include <string>
#include <OpenSG/OSGConfig.h>

namespace Gtk {
    class Fixed;
    class Separator;
}

using namespace std;
OSG_BEGIN_NAMESPACE;

struct VRConnectorWidget {
    Gtk::Separator* sh1 = 0;
    Gtk::Separator* sh2 = 0;
    Gtk::Separator* sv1 = 0;
    Gtk::Separator* sv2 = 0;
    Gtk::Fixed* canvas = 0;
    VRSemanticWidgetWeakPtr w1;
    VRSemanticWidgetWeakPtr w2;
    bool visible = true;

    VRConnectorWidget(Gtk::Fixed* canvas, string color);
    ~VRConnectorWidget();

    void set(VRSemanticWidgetPtr w1, VRSemanticWidgetPtr w2);
    void setVisible(bool visible = true);
    void update();
};

OSG_END_NAMESPACE;

#endif // VRCONNECTORWIDGET_H_INCLUDED
