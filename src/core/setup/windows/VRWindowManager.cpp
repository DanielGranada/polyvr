#include "VRWindowManager.h"
#include "core/setup/devices/VRMouse.h"
#include "core/setup/devices/VRKeyboard.h"
#include "core/gui/VRGuiUtils.h"
#include <libxml++/nodes/element.h>
#include <gtkmm/builder.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>
#include "core/utils/VROptions.h"
#include "core/utils/VRTimer.h"
#include "core/objects/object/VRObject.h"
#include "core/utils/VRRate.h"
#include "core/scene/VRScene.h"
#include "core/gui/VRGuiManager.h"
#include "core/utils/VRFunction.h"
#include "core/utils/VRGlobals.h"

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGGLUTWindow.h>

#include "VRView.h"
#include "VRGlutWindow.h"
#include "VRGtkWindow.h"
#include "VRMultiWindow.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

VRWindowManager::VRWindowManager() {
    cout << "Init VRWindowManager\n";
    ract = RenderAction::create();
}

VRWindowManager::~VRWindowManager() {
    windows.clear();
    BarrierRefPtr barrier = Barrier::get("PVR_rendering", true);
    for (uint i=0; i<VRWindow::active_window_count+1; i++) subRef(barrier);
}

bool VRWindowManager::checkWin(string name) {
    if (windows.count(name) == 1) return true;
    else {
        cout << "\nError! request for not existing window " << name << "\n";
        return false;
    }
}

RenderActionRefPtr VRWindowManager::getRenderAction() { return ract; }

void setMultisampling(bool on) {
    bool res = false;

#if !defined(GL_MULTISAMPLE_SGIS) && !defined(GL_MULTISAMPLE_ARB)
#ifndef _WIN32
#warning "No Multisampling support detected, disabling"
#endif
#else

  if (on) {
#ifdef GL_MULTISAMPLE_SGIS
    res = true;
    glEnable(GL_MULTISAMPLE_SGIS);
#endif //GL_MULTISAMPLE_SGIS
#ifdef GL_MULTISAMPLE_ARB
    res = true;
    glEnable(GL_MULTISAMPLE_ARB);
#endif //GL_MULTISAMPLE_ARB
  } else {
#ifdef GL_MULTISAMPLE_SGIS
    glDisable(GL_MULTISAMPLE_SGIS);
#endif //GL_MULTISAMPLE_SGIS
#ifdef GL_MULTISAMPLE_ARB
    glDisable(GL_MULTISAMPLE_ARB);
#endif //GL_MULTISAMPLE_ARB
  }
#endif //!defined(GL_MULTISAMPLE_SGIS) && !defined(GL_MULTISAMPLE_ARB)
    //cout << "\nSET AA: " << res;
    if (res) {;};//__GL_FXAA_MODE 	 = 7;//find out how to set vie application
}

void VRWindowManager::initGlut() { // deprecated?
    static bool initiated = false;
    if (initiated) return;
    initiated = true;

    return;

    cout << "\nGlut: Init\n";
    int argc = 0;
    glutInit(&argc, NULL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //setMultisampling(true);

    if (VROptions::get()->getOption<bool>("active_stereo"))
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STEREO | GLUT_STENCIL);
    else glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);
}

VRWindowPtr VRWindowManager::addGlutWindow(string name) {
    initGlut();
    VRGlutWindowPtr win = VRGlutWindow::create();
    win->setName(name);
    win->setAction(ract);
    windows[win->getName()] = win;
    return win;
}

VRWindowPtr VRWindowManager::addMultiWindow(string name) {
    VRMultiWindowPtr win = VRMultiWindow::create();
    win->setName(name);
    win->setAction(ract);
    windows[win->getName()] = win;
    return win;
}

VRWindowPtr VRWindowManager::addGtkWindow(string name, string glarea) {
    cout << " add Gtk window " << name << endl;
    Gtk::DrawingArea* drawArea = 0;
    VRGuiBuilder()->get_widget(glarea, drawArea); // TODO: create new glarea, add flag to editor area window!
    VRGtkWindowPtr win = VRGtkWindow::create(drawArea);
    editorWindow = win;
    win->setName(name);
    win->setAction(ract);
    windows[win->getName()] = win;
    return win;
}

VRGtkWindowPtr VRWindowManager::getEditorWindow() { return editorWindow; }

void VRWindowManager::pauseRendering(bool b) { rendering_paused = b; }

void VRWindowManager::getWindowSize(string name, int& width, int& height) {
    if (!checkWin(name)) return;

    WindowRecPtr win = windows[name]->getOSGWindow();
    width = win->getWidth();
    height = win->getHeight();
}

void VRWindowManager::updateWindows() {
    if (rendering_paused) return;
    auto scene = VRScene::getCurrent();
    if (scene) scene->allowScriptThreads();

    ract->setResetStatistics(false);
    StatCollector* sc = ract->getStatCollector();
    if (sc) {
        sc->reset();
        sc->getElem(VRGlobals::FRAME_RATE.statFPS)->add(VRGlobals::FRAME_RATE.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP1.statFPS)->add(VRGlobals::UPDATE_LOOP1.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP2.statFPS)->add(VRGlobals::UPDATE_LOOP2.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP3.statFPS)->add(VRGlobals::UPDATE_LOOP3.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP4.statFPS)->add(VRGlobals::UPDATE_LOOP4.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP5.statFPS)->add(VRGlobals::UPDATE_LOOP5.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP6.statFPS)->add(VRGlobals::UPDATE_LOOP6.fps);
        sc->getElem(VRGlobals::UPDATE_LOOP7.statFPS)->add(VRGlobals::UPDATE_LOOP7.fps);
        sc->getElem(VRGlobals::RENDER_FRAME_RATE.statFPS)->add(VRGlobals::RENDER_FRAME_RATE.fps);
        sc->getElem(VRGlobals::SLEEP_FRAME_RATE.statFPS)->add(VRGlobals::SLEEP_FRAME_RATE.fps);
        sc->getElem(VRGlobals::SWAPB_FRAME_RATE.statFPS)->add(VRGlobals::SWAPB_FRAME_RATE.fps);
        sc->getElem(VRGlobals::WINDOWS_FRAME_RATE.statFPS)->add(VRGlobals::WINDOWS_FRAME_RATE.fps);
        sc->getElem(VRGlobals::SCRIPTS_FRAME_RATE.statFPS)->add(VRGlobals::SCRIPTS_FRAME_RATE.fps);
        sc->getElem(VRGlobals::PHYSICS_FRAME_RATE.statFPS)->add(VRGlobals::PHYSICS_FRAME_RATE.fps);
        sc->getElem(VRGlobals::GTK1_FRAME_RATE.statFPS)->add(VRGlobals::GTK1_FRAME_RATE.fps);
        sc->getElem(VRGlobals::GTK2_FRAME_RATE.statFPS)->add(VRGlobals::GTK2_FRAME_RATE.fps);
        sc->getElem(VRGlobals::SMCALLBACKS_FRAME_RATE.statFPS)->add(VRGlobals::SMCALLBACKS_FRAME_RATE.fps);
        sc->getElem(VRGlobals::SETUP_FRAME_RATE.statFPS)->add(VRGlobals::SETUP_FRAME_RATE.fps);
        sc->getElem(VRGlobals::SCRIPTS_FRAME_RATE.statFPS)->add(VRGlobals::SCRIPTS_FRAME_RATE.fps);
    }

    commitChanges();

    BarrierRefPtr barrier = Barrier::get("PVR_rendering", true);
    if (barrier->getNumWaiting() == VRWindow::active_window_count) {
        barrier->enter(VRWindow::active_window_count+1);
        for (auto w : getWindows() ) if (auto win = dynamic_pointer_cast<VRGtkWindow>(w.second)) win->render();
        barrier->enter(VRWindow::active_window_count+1);
    }

    Thread::getCurrentChangeList()->clear();
    if (scene) scene->blockScriptThreads();
}

void VRWindowManager::setWindowView(string name, VRViewPtr view) {
    if (!checkWin(name)) return;
    VRWindowPtr win = windows[name];
    win->addView(view);
}

void VRWindowManager::addWindowServer(string name, string server) {
    if (!checkWin(name)) return;
    VRMultiWindowPtr win = dynamic_pointer_cast<VRMultiWindow>( windows[name] );
    win->addServer(server);
}

void VRWindowManager::removeWindow(string name) { windows.erase(name); }

void VRWindowManager::changeWindowName(string& name, string new_name) {
    map<string, VRWindowPtr>::iterator i = windows.find(name);
    if (i == windows.end()) return;

    VRWindowPtr win = i->second;
    windows.erase(i);
    win->setName(new_name);
    windows[win->getName()] = win;
    name = win->getName();
}

map<string, VRWindowPtr> VRWindowManager::getWindows() { return windows; }

VRWindowPtr VRWindowManager::getWindow(string name) {
    if (windows.count(name) == 0) return 0;
    else return windows[name];
}

void VRWindowManager::save(xmlpp::Element* node) {
    map<string, VRWindowPtr>::iterator itr;
    xmlpp::Element* wn;
    for (itr = windows.begin(); itr != windows.end(); itr++) {
        wn = node->add_child("Window");
        itr->second->save(wn);
    }
}

void VRWindowManager::load(xmlpp::Element* node) {
    //e->get_attribute("from")->get_value();
    xmlpp::Node::NodeList nl = node->get_children();
    xmlpp::Node::NodeList::iterator itr;
    for (itr = nl.begin(); itr != nl.end(); itr++) {
        xmlpp::Node* n = *itr;

        xmlpp::Element* el = dynamic_cast<xmlpp::Element*>(n);
        if (!el) continue;

        string type = el->get_attribute("type")->get_value();
        string name = el->get_attribute("name")->get_value();

        if (type == "0") {
            VRWindowPtr win = addMultiWindow(name);
            win->load(el);
        }

        if (type == "2") {
            VRWindowPtr win = addGtkWindow(name);
            win->load(el);
        }
    }
}

OSG_END_NAMESPACE;
