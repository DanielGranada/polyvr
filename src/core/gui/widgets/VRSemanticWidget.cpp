#include "VRSemanticWidget.h"
#include "VRConceptWidget.h"
#include "VREntityWidget.h"
#include "VRRuleWidget.h"
#include "../VRGuiSemantics.h"

#include <gtkmm/object.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/stock.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/fixed.h>
#include <gtkmm/expander.h>

using namespace OSG;

void VRSemanticWidget_on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& data, unsigned int info, unsigned int time, VRSemanticWidget* e) {
    data.set("concept", 0, (const guint8*)&e, sizeof(void*));
}

void VRSemanticWidget_on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& data, guint info, guint time, VRSemanticWidget* self) {
    if (data.get_target() != "concept") { cout << "VRGuiSemantics_on_drag_data_received, wrong dnd: " << data.get_target() << endl; return; }
    VRSemanticWidget* e = *(VRSemanticWidget**)data.get_data();
    self->reparent(e->ptr());
}

VRSemanticWidget::VRSemanticWidget(VRGuiSemantics* m, Gtk::Fixed* canvas, string color) {
    this->canvas = canvas;
    manager = m;

    // properties treeview
    VRGuiSemantics_PropsColumns cols;
    auto treestore = Gtk::TreeStore::create(cols);
    treeview = Gtk::manage( new Gtk::TreeView() );
    treeview->set_model(treestore);
    treeview->set_headers_visible(false);
    treeview->signal_cursor_changed().connect( sigc::mem_fun(*this, &VRSemanticWidget::on_select_property) );

    auto addMarkupColumn = [&](string title, Gtk::TreeModelColumn<Glib::ustring>& col, bool editable = false) {
        Gtk::CellRendererText* renderer = Gtk::manage(new Gtk::CellRendererText());
        renderer->property_editable() = editable;
        Gtk::TreeViewColumn* column = Gtk::manage(new Gtk::TreeViewColumn(title, *renderer));
        column->add_attribute(renderer->property_markup(), col);
        treeview->append_column(*column);
    };

    addMarkupColumn(" Properties:", cols.name, true);
    addMarkupColumn("", cols.type);

    // buttons
    auto toolbar = Gtk::manage( new Gtk::Toolbar() );
    toolbar->set_icon_size(Gtk::ICON_SIZE_MENU);
    toolbar->set_show_arrow(0);

    auto addButton = [&](Gtk::BuiltinStockID icon, string tooltip, void (VRSemanticWidget::* cb)()) {
        auto b = Gtk::manage( new Gtk::ToolButton(icon) );
        toolbar->add(*b);
        b->set_tooltip_text(tooltip);
        b->signal_clicked().connect( sigc::mem_fun(*this, cb) );
    };

    addButton(Gtk::Stock::ADD, "Fold/unfold subtree", &VRSemanticWidget::on_fold_clicked); // Gtk::Stock::REMOVE
    addButton(Gtk::Stock::EDIT, "edit concept name", &VRSemanticWidget::on_edit_clicked);
    addButton(Gtk::Stock::CLOSE, "remove concept", &VRSemanticWidget::on_rem_clicked);
    auto sep = Gtk::manage( new Gtk::SeparatorToolItem() ); toolbar->add(*sep); // separator
    addButton(Gtk::Stock::NEW, "new property", &VRSemanticWidget::on_newp_clicked);
    addButton(Gtk::Stock::EDIT, "edit selected property", &VRSemanticWidget::on_edit_prop_clicked);
    addButton(Gtk::Stock::DELETE, "remove selected property", &VRSemanticWidget::on_rem_prop_clicked);

    // expander and frame
    auto vbox = Gtk::manage( new Gtk::VBox() );
    toolbars = Gtk::manage( new Gtk::HBox() );
    auto expander = Gtk::manage( new Gtk::Expander("") );
    label = (Gtk::Label*)expander->get_label_widget();
    expander->add(*vbox);
    toolbars->pack_start(*toolbar);
    vbox->pack_start(*toolbars);
    vbox->pack_start(*treeview);
    auto frame = Gtk::manage( new Gtk::Frame() );
    frame->add(*expander);
    widget = frame;
    canvas->put(*frame, 0, 0);
    frame->modify_bg( Gtk::STATE_NORMAL, Gdk::Color(color));

    // dnd drag widget
    vector<Gtk::TargetEntry> entries;
    entries.push_back(Gtk::TargetEntry("concept", Gtk::TARGET_SAME_APP));
    expander->drag_source_set(entries, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
    expander->signal_drag_data_get().connect( sigc::bind<VRSemanticWidget*>( sigc::ptr_fun(VRSemanticWidget_on_drag_data_get), this ) );

    // dnd drop on widget
    expander->drag_dest_set(entries, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
    expander->signal_drag_data_received().connect( sigc::bind<VRSemanticWidget*>( sigc::ptr_fun(VRSemanticWidget_on_drag_data_received), this ) );
}

VRSemanticWidget::~VRSemanticWidget() {
    if (widget->get_parent() == canvas) {
        canvas->remove(*widget);
        canvas->show_all();
    }
}

VRSemanticWidgetPtr VRSemanticWidget::ptr() { return shared_from_this(); }

void VRSemanticWidget::reparent(VRSemanticWidgetPtr w) {
    auto c = dynamic_pointer_cast<VRConceptWidget>(w);
    auto e = dynamic_pointer_cast<VREntityWidget>(w);
    auto r = dynamic_pointer_cast<VRRuleWidget>(w);
    if (c) reparent(c);
    if (e) reparent(e);
    if (r) reparent(r);
}

void VRSemanticWidget::setPropRow(Gtk::TreeModel::iterator iter, string name, string type, string color, int flag, int ID, int rtype) {
    string cname = "<span color=\""+color+"\">" + name + "</span>";
    string ctype = "<span color=\""+color+"\">" + type + "</span>";

    Gtk::TreeStore::Row row = *iter;
    Glib::RefPtr<Gtk::TreeStore> treestore = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic( treeview->get_model() );

    gtk_tree_store_set(treestore->gobj(), row.gobj(), 0, cname.c_str(), -1);
    gtk_tree_store_set(treestore->gobj(), row.gobj(), 1, ctype.c_str(), -1);
    gtk_tree_store_set(treestore->gobj(), row.gobj(), 2, name.c_str(), -1);
    gtk_tree_store_set(treestore->gobj(), row.gobj(), 3, type.c_str(), -1);
    gtk_tree_store_set(treestore->gobj(), row.gobj(), 4, flag, -1);
    gtk_tree_store_set(treestore->gobj(), row.gobj(), 5, rtype, -1);
    gtk_tree_store_set(treestore->gobj(), row.gobj(), 6, ID, -1);
}

bool VRSemanticWidget::on_expander_clicked(GdkEventButton* e) {
    cout << "EXPAND\n";
    return true;
}

void VRSemanticWidget::on_select() {}

void VRSemanticWidget::on_fold_clicked() {
    subTreeFolded = !subTreeFolded;
    if (subTreeFolded) {
        // TODO: change button icon
    }
    setFolding(subTreeFolded);
}

void VRSemanticWidget::setFolding(bool folded) { // map, unmap
    subTreeFolded = folded;
    for (auto ww : children) {
        if (auto w = ww.second.lock()) {
            w->setVisible(!subTreeFolded);
            w->setFolding(subTreeFolded);
        }
    }
}

void VRSemanticWidget::setVisible(bool v) { // map, unmap
    visible = v;
    if (v) widget->map();
    else widget->unmap();
}

void VRSemanticWidget::move(Vec2f p) {
    pos = p;
    float w = widget->get_width();
    float h = widget->get_height();
    canvas->move(*widget, p[0]-w*0.5, p[1]-h*0.5);
}

Vec3f VRSemanticWidget::getPosition() { return Vec3f(pos[0], pos[1], 0); }
Vec3f VRSemanticWidget::getSize() { return Vec3f(widget->get_width()*0.5, widget->get_height()*0.5, 0); }

Vec2f VRSemanticWidget::getAnchorPoint(Vec2f p) {
    float w = abs(p[0]-pos[0]);
    float h = abs(p[1]-pos[1]);
    if (w >= h && p[0] < pos[0]) return pos - Vec2f(widget->get_width()*0.5, 0);
    if (w >= h && p[0] > pos[0]) return pos + Vec2f(widget->get_width()*0.5, 0);
    if (w < h && p[1] < pos[1]) return pos - Vec2f(0, widget->get_height()*0.5);
    if (w < h && p[1] > pos[1]) return pos + Vec2f(0, widget->get_height()*0.5);
    return pos;
}
