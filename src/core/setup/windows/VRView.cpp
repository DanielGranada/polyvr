#include "VRView.h"
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGImageForeground.h>
#include <libxml++/nodes/element.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGMultiPassMaterial.h>

#include "core/math/pose.h"
#include "core/utils/VRRate.h"
#include "core/utils/toString.h"
#include "core/utils/VRGlobals.h"
#include "core/tools/VRText.h"
#include "core/setup/VRSetup.h"
#include "core/setup/devices/VRMultiTouch.h"
#include "core/gui/VRGuiUtils.h"
#include "core/gui/VRGuiManager.h"
#include "core/objects/OSGObject.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/material/OSGMaterial.h"
#include "core/objects/material/VRTexture.h"
#include "core/objects/geometry/VRSprite.h"
#include "core/objects/VRTransform.h"
#include "core/objects/VRCamera.h"
#include "core/objects/OSGCamera.h"
#include "core/objects/object/VRObjectT.h"
#include "core/objects/VRLight.h"
#include "core/scene/VRSceneManager.h"
#include "core/scene/rendering/VRRenderStudio.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

bool onBox(int i, int j, int c) {
    if(abs(i) > c || abs(j) > c) return false;
    if(abs(i) == c || abs(j) == c) return true;
    return false;
}

string VRView::getName() { return name; }

void VRView::setMaterial() {
    ImageRecPtr img = Image::create();

    Vec3d bg  = Vec3d(0.5, 0.7, 0.95);
    Vec3d c1  = Vec3d(0.5, 0.7, 0.95);
    Vec3d cax = Vec3d(0.9, 0.2, 0.2);
    Vec3d cay = Vec3d(0.2, 0.9, 0.2);

    auto label = VRText::get()->create(name, "SANS 20", 20, Color4f(1,1,1,1), Color4f(bg[2], bg[1], bg[0], 1));
    float lw = label->getImage()->getWidth();
    float lh = label->getImage()->getHeight();

    int s=256;
    int b1 = 0.5*s-8;
    int b2 = 0.5*s-50;
    int ar = 50;
    Vec2d pl(-0.1*s, -0.05*s);

	vector<Vec3d> data;
	data.resize(s*s);

    for (int i=0; i<s; i++) {
        for (int j=0; j<s; j++) {
            int k = i+j*s;
            data[k] = bg;

            int x = i-0.5*s;
            int y = j-0.5*s;

            if (onBox(x,y,b1)) data[k] = c1; // box1
            if (onBox(x,y,b2)) data[k] = c1; // box2

            if (y == 0 && x >= 0 && x < ar) data[k] = cax; // ax
            if (x == 0 && y >= 0 && y < ar) data[k] = cay; // ax

            if (x >= pl[0]-lw*0.5 && x < pl[0]+lw*0.5 && y >= pl[1]-lh*0.5 && y < pl[1]+lh*0.5) {
                int u = x - pl[0] + lw*0.5;
                int v = y - pl[1] + lh*0.5;
                int w = 4*(u+v*lw);
                const UInt8* d = label->getImage()->getData();
                data[k] = Vec3d(d[w]/255.0, d[w+1]/255.0, d[w+2]/255.0);
                //data[k] = Vec4d(1,1,1, 1);
            }
        }
    }

    img->set( Image::OSG_RGB_PF, s, s, 1, 0, 1, 0, (const uint8_t*)&data[0], OSG::Image::OSG_FLOAT32_IMAGEDATA, true, 1);

    viewGeoMat->setTexture(VRTexture::create(img));
    viewGeoMat->setLit(false);
}

void VRView::setViewports() {//create && set size of viewports
    if (window && lView) window->subPortByObj(lView);
    if (window && rView) window->subPortByObj(rView);
    lView = 0;
    rView = 0;

    Vec4d p = position;
    p[1] = 1-position[3]; // invert y
    p[3] = 1-position[1];

    if (p[0] > p[2]) p[0] = p[2]-0.01;
    if (p[1] > p[3]) p[1] = p[3]-0.01;

    //active, stereo
    lView_act = active_stereo ? StereoBufferViewportRecPtr(StereoBufferViewport::create()) : 0;
    rView_act = active_stereo ? StereoBufferViewportRecPtr(StereoBufferViewport::create()) : 0;

    //no stereo
    if (!stereo && !active_stereo) {
        lView = Viewport::create();
        lView->setSize(p[0], p[1], p[2], p[3]);
        rView = 0;
    }

    if (stereo && !active_stereo) {
        lView = Viewport::create();
        rView = Viewport::create();
        // left bottom right top
        lView->setSize(p[0], p[1], (p[0]+p[2])*0.5, p[3]);
        rView->setSize((p[0]+p[2])*0.5, p[1], p[2], p[3]);

        // special test config that induces strange artifacts for ssao
        //lView->setSize(0, 0, 0.5, 1.0);
        //rView->setSize(0.1, 0, 1.0, 0.5);
    }

    if (active_stereo) {
        lView_act->setLeftBuffer(true);
        lView_act->setRightBuffer(false);
        rView_act->setLeftBuffer(false);
        rView_act->setRightBuffer(true);

        lView = lView_act;
        rView = rView_act;

        lView->setSize(p[0], p[1], p[2], p[3]);
        rView->setSize(p[0], p[1], p[2], p[3]);
    }

    // renderingL stages
    if (renderingL) {
        renderingL->setHMDDeye(-1);
        renderingR->setHMDDeye(1);
        Vec2i s = window_size;
        float w = abs(p[3]-p[1]);
        float h = abs(p[2]-p[0]);
        s = Vec2i(s[0]*w, s[1]*h);
        if (!stereo && !active_stereo) renderingL->resize(s);
        else {
            s[0] *= 0.5;
            renderingL->resize(s);
            renderingR->resize(s);
        }
    }
}

void VRView::setBG() {
    if (background) {
        if (lView) lView->setBackground(background);
        if (rView) rView->setBackground(background);
        if (renderingL) renderingL->setBackground(background);
        if (renderingR) renderingR->setBackground(background);
    }
}

void VRView::setDecorators() {//set decorators, only if projection true
    if (projection) { // put in setProjection fkt

        //View
        proj_normal.normalize();
        proj_up.normalize();
        float w = proj_size[0];
        float h = proj_size[1];
        Vec3d x = proj_normal.cross(proj_up);

        screenLowerLeft = Pnt3f(offset + proj_center - proj_up*h*(0.5+proj_warp[1]+proj_shear[1]) + x*w*(0.5-proj_warp[0]+proj_shear[0]));
        screenLowerRight = Pnt3f(offset + proj_center - proj_up*h*(0.5-proj_warp[1]-proj_shear[1]) - x*w*(0.5-proj_warp[0]-proj_shear[0]));
        screenUpperRight = Pnt3f(offset + proj_center + proj_up*h*(0.5-proj_warp[1]+proj_shear[1]) - x*w*(0.5+proj_warp[0]+proj_shear[0]));
        screenUpperLeft = Pnt3f(offset + proj_center + proj_up*h*(0.5+proj_warp[1]-proj_shear[1]) + x*w*(0.5+proj_warp[0]-proj_shear[0]));
        if (getUser()) getUser()->setFrom(proj_user);
    } else {
        screenLowerLeft = Pnt3f(-1,-0.6, -1);
        screenLowerRight = Pnt3f(1,-0.6, -1);
        screenUpperRight = Pnt3f(1,0.6, -1);
        screenUpperLeft = Pnt3f(-1,0.6, -1);
    }

    //cout << "setDecorator screen: shear " << proj_shear << " warp " << proj_warp << endl;
    //cout << "setDecorator screen: LL " << screenLowerLeft << " LR " << screenLowerRight << " UR " << screenUpperRight << " UL " << screenUpperLeft << " " << endl;

    GeometryMTRecPtr geo = dynamic_cast<Geometry*>(viewGeo->getCore());
    GeoVectorPropertyRecPtr pos = geo->getPositions();

    pos->setValue(screenLowerLeft, 0);
    pos->setValue(screenLowerRight, 1);
    pos->setValue(screenUpperLeft, 2);
    pos->setValue(screenUpperRight, 3);

    if (!projection && !stereo) {
        PCDecoratorLeft = 0;
        PCDecoratorRight = 0;
        return;
    }

    if (projection && !stereo) {
        cout << "\nset single projection decorator";
        PCDecoratorLeft = ProjectionCameraDecorator::create();
        PCDecoratorLeft->setLeftEye(true);
        PCDecoratorLeft->setEyeSeparation(0);
        PCDecoratorLeft->editMFSurface()->clear();
        PCDecoratorLeft->editMFSurface()->push_back(screenLowerLeft);
        PCDecoratorLeft->editMFSurface()->push_back(screenLowerRight);
        PCDecoratorLeft->editMFSurface()->push_back(screenUpperRight);
        PCDecoratorLeft->editMFSurface()->push_back(screenUpperLeft);
        PCDecoratorRight = 0;
        return;
    }

    // stereo

    cout << "\nset projection decorators";
    PCDecoratorLeft = ProjectionCameraDecorator::create();
    PCDecoratorRight = ProjectionCameraDecorator::create();

    PCDecoratorLeft->setLeftEye(false);
    PCDecoratorLeft->setEyeSeparation(0.06);
    PCDecoratorLeft->editMFSurface()->clear();
    PCDecoratorLeft->editMFSurface()->push_back(screenLowerLeft);
    PCDecoratorLeft->editMFSurface()->push_back(screenLowerRight);
    PCDecoratorLeft->editMFSurface()->push_back(screenUpperRight);
    PCDecoratorLeft->editMFSurface()->push_back(screenUpperLeft);

    PCDecoratorRight->setLeftEye(true);
    PCDecoratorRight->setEyeSeparation(0.06);
    PCDecoratorRight->editMFSurface()->clear();
    PCDecoratorRight->editMFSurface()->push_back(screenLowerLeft);
    PCDecoratorRight->editMFSurface()->push_back(screenLowerRight);
    PCDecoratorRight->editMFSurface()->push_back(screenUpperRight);
    PCDecoratorRight->editMFSurface()->push_back(screenUpperLeft);
}

VRView::VRView(string name) {
    this->name = name;

    SolidBackgroundRecPtr sbg = SolidBackground::create();
    sbg->setColor(Color3f(0.7, 0.7, 0.7));
    background = sbg;

    dummy_user = VRTransform::create("view_user");
    dummy_user->setPersistency(0);

    viewGeo = makeNodeFor(makePlaneGeo(1,1,1,1));
    viewGeo->setTravMask(0);
    viewGeoMat = VRMaterial::create("setup view mat");
    GeometryMTRecPtr geo = dynamic_cast<Geometry*>( viewGeo->getCore() );
    geo->setMaterial(viewGeoMat->getMaterial()->mat);

    renderingL = VRRenderStudio::create( VRRenderStudio::LEFT );
    renderingR = VRRenderStudio::create( VRRenderStudio::RIGHT );
    renderingL->init();
    renderingR->init();

    update();
}

VRView::~VRView() {
    window->subPortByObj(lView);
    window->subPortByObj(rView);
    window->subPortByObj(lView_act);
    window->subPortByObj(rView_act);
    lView = 0;
    rView = 0;
    lView_act = 0;
    rView_act = 0;
    PCDecoratorLeft = 0;
    PCDecoratorRight = 0;
    stats = 0;
}

VRViewPtr VRView::create(string name) { return VRViewPtr(new VRView(name)); }
VRViewPtr VRView::ptr() { return shared_from_this(); }

VRRenderStudioPtr VRView::getRenderingL() { return renderingL; }
VRRenderStudioPtr VRView::getRenderingR() { return renderingR; }

void VRView::setSize(Vec2i s) { window_size = s; update(); }
Vec2i VRView::getSize() { return window_size; }

int VRView::getID() { return ID; }
void VRView::setID(int i) { ID = i; }

void VRView::showStats(bool b) {
    if (stats == 0) {
        stats = SimpleStatisticsForeground::create();
        stats->setSize(25);
        stats->setColor(Color4f(0.1,0.9,0.6,0.7f));

        stats->addText("\nPerformance:");
        stats->addElement(VRGlobals::FRAME_RATE.statFPS, " application FPS: %d");
        stats->addElement(VRGlobals::UPDATE_LOOP1.statFPS, "  main update loop point 1 FPS (after first Gtk update) : %d");
        stats->addElement(VRGlobals::UPDATE_LOOP2.statFPS, "  main update loop point 2 FPS (after callbacks update) : %d");
        stats->addElement(VRGlobals::UPDATE_LOOP3.statFPS, "  main update loop point 3 FPS (after hardware update)  : %d");
        stats->addElement(VRGlobals::UPDATE_LOOP4.statFPS, "  main update loop point 4 FPS (after scene update)     : %d");
        stats->addElement(VRGlobals::UPDATE_LOOP5.statFPS, "  main update loop point 5 FPS (after remote rendering) : %d");
        stats->addElement(VRGlobals::UPDATE_LOOP6.statFPS, "  main update loop point 6 FPS (after second Gtk update): %d");
        stats->addElement(VRGlobals::UPDATE_LOOP7.statFPS, "  main update loop point 7 FPS (end of loop iteration)  : %d");
        stats->addElement(VRGlobals::SLEEP_FRAME_RATE.statFPS, " application sleep FPS: %d");
        stats->addElement(VRGlobals::SLEEP_FRAME_RATE.statFPS, " application sleep FPS: %d");
        stats->addElement(VRGlobals::SCRIPTS_FRAME_RATE.statFPS, "  script FPS: %d");
        stats->addElement(VRGlobals::WINDOWS_FRAME_RATE.statFPS, "  distributed windows FPS: %d");
        stats->addElement(VRGlobals::GTK1_FRAME_RATE.statFPS, " GTK devices FPS: %d");
        stats->addElement(VRGlobals::GTK2_FRAME_RATE.statFPS, " GTK rendering FPS: %d");
        stats->addElement(VRGlobals::RENDER_FRAME_RATE.statFPS, "  rendering FPS: %d");
        stats->addElement(VRGlobals::SWAPB_FRAME_RATE.statFPS, "  swap buffer FPS: %d");
        stats->addElement(RenderAction::statDrawTime, "   draw FPS: %r.2f");
        stats->addElement(RenderAction::statTravTime, "   trav FPS: %r.2f");
        stats->addElement(VRGlobals::SMCALLBACKS_FRAME_RATE.statFPS, " scene manager callbacks FPS: %d");
        stats->addElement(VRGlobals::SETUP_FRAME_RATE.statFPS, " setup devices FPS: %d");
        stats->addElement(VRGlobals::PHYSICS_FRAME_RATE.statFPS, " physics FPS: %d");

        stats->addText("\nMaterials:");
        stats->addElement(RenderAction::statNStates, " state changes: %d");
        stats->addElement(RenderAction::statNShaders, " shader changes: %d");
        stats->addElement(RenderAction::statNShaderParams, " shader param changes: %d");
        stats->addElement(TextureObjChunk::statNTextures, " textures: %d");
        stats->addElement(TextureObjChunk::statNTexBytes, " tex mem: %MB MB");
        stats->addElement(RenderAction::statNChunks, " chunk changes: %d");

        stats->addText("\nScene:");
        stats->addElement(RenderAction::statNMatrices, " matrix changes: %d");
        stats->addElement(Drawable::statNTriangles, " tris: %d");
        stats->addElement(Drawable::statNLines, " lines: %d");
        stats->addElement(Drawable::statNPoints, " points: %d");
        stats->addElement(Drawable::statNVertices, " verts: %d");

        stats->addText("\nChange list:");
        stats->addElement(ChangeList::statNChangedStoreSize, " %d entries in changedStore");
        stats->addElement(ChangeList::statNCreatedStoreSize, " %d entries in createdStore");
        stats->addElement(ChangeList::statNUnCommittedStoreSize, " %d entries in uncommitedStore");
        stats->addElement(ChangeList::statNPoolSize, " %d entries in pool");


        //stats->addElement(RenderAction::statNGeometries, "    Geom nodes: %d");
        //stats->addElement(RenderAction::statNTransGeometries, "Transparent Nodes drawn   %d");
        //stats->addElement(RenderAction::statNTriangles, "     Triangles: %d");
        //stats->addElement(RenderAction::statNMaterials, "%d material changes");
        //stats->addElement(PointLight::statNPointLights, "%d active point lights");
        //stats->addElement(DirectionalLight::statNDirectionalLights, "%d active directional lights");
        //stats->addElement(SpotLight::statNSpotLights, "%d active spot lights");
        //stats->addElement(DrawActionBase::statCullTestedNodes, "Nodes culltested      %d");
        //stats->addElement(DrawActionBase::statCulledNodes, "Nodes culled          %d");
        //stats->addElement(RenderAction::statNOcclusionMode, "Occlusion culling     %s");
        //stats->addElement(RenderAction::statNOcclusionTests, "Occlusion tests       %d");
        //stats->addElement(RenderAction::statNOcclusionCulled, "Occlusion culled      %d");

        if (stats->getCollector() != NULL) stats->getCollector() ->getElem(Drawable::statNTriangles);
        if (stats->getCollector() != NULL) stats->editCollector()->getElem(Drawable::statNTriangles);
    }

    if (lView == 0) return;
    if (b && !doStats) lView->addForeground(stats);
    if (!b && doStats) lView->removeObjFromForegrounds(stats);
    doStats = b;

    VRSetup::getCurrent()->getRenderAction()->setStatCollector(stats->getCollector());
}

void VRView::showViewGeo(bool b) {
    if (b) viewGeo->setTravMask(0xffffffff);
    else viewGeo->setTravMask(0);
}

Vec4d VRView::getPosition() { return position; }
void VRView::setPosition(Vec4d pos) { position = pos; update(); }

void VRView::setRoot(VRObjectPtr root, VRTransformPtr real) { view_root = root; real_root = real; update(); }

void VRView::setRoot() {
    if (real_root && viewGeo) real_root->addChild(OSGObject::create(viewGeo));

    if (user && real_root) user->switchParent(real_root);
    if (dummy_user && real_root) dummy_user->switchParent(real_root);

    NodeMTRecPtr nl = view_root ? view_root->getNode()->node : 0;
    NodeMTRecPtr nr = nl;

    if (renderingL) {
        renderingL->setScene(view_root);
        nl = renderingL->getRoot()->getNode()->node;
        renderingL->getRoot()->setVolumeCheck( false, true );
    }

    if (renderingR) {
        renderingR->setScene(view_root);
        nr = renderingR->getRoot()->getNode()->node;
        renderingR->getRoot()->setVolumeCheck( false, true );
    }

    if (lView) lView->setRoot(nl);
    if (rView) rView->setRoot(nr);
}

void VRView::setMirror(bool b) { mirror = b; update(); }
void VRView::setMirrorPos(Vec3d p) { mirrorPos = p; updateMirrorMatrix(); }
void VRView::setMirrorNorm(Vec3d n) { mirrorNorm = n; updateMirrorMatrix(); }

void VRView::updateMirrorMatrix() {
    Matrix4d Z, mI;
    Z.setScale(Vec3d(1,1,-1));
    auto m = Pose(mirrorPos, mirrorNorm).asMatrix();
    m.inverse(mI);
    mirrorMatrix = mI;
    mirrorMatrix.mult(Z);
    mirrorMatrix.mult(m);
}

bool VRView::getMirror() { return mirror; }
Vec3d VRView::getMirrorPos() { return mirrorPos; }
Vec3d VRView::getMirrorNorm() { return mirrorNorm; }

void VRView::updateMirror() {
    if (!mirror || !user) return;
    auto u = user->getMatrix();
    u.multLeft(mirrorMatrix); // u' = m*Z*mI*u
    dummy_user->setMatrix(u);
}

void VRView::setUser(VRTransformPtr u) {
    user = u;
    user_name = user ? user->getName() : "";
    update();
}

void VRView::setUser() {
    if (user == 0 && user_name != "") user = VRSetup::getCurrent()->getTracker(user_name);

    if (user == 0 || mirror) {
        if (PCDecoratorLeft) PCDecoratorLeft->setUser(dummy_user->getNode()->node);
        if (PCDecoratorRight) PCDecoratorRight->setUser(dummy_user->getNode()->node);
    } else {
        user_name = user->getName();
        if (PCDecoratorLeft) PCDecoratorLeft->setUser(user->getNode()->node);
        if (PCDecoratorRight) PCDecoratorRight->setUser(user->getNode()->node);
    }
}

void VRView::setCamera(VRCameraPtr c) {
    cam = c;
    update();
}

void VRView::setCam() {
    if (cam == 0) return;

    if (lView && PCDecoratorLeft == 0) lView->setCamera(cam->getCam()->cam);
    if (rView && PCDecoratorRight == 0) rView->setCamera(cam->getCam()->cam);

    if (PCDecoratorLeft) PCDecoratorLeft->setDecoratee(cam->getCam()->cam);
    if (PCDecoratorRight) PCDecoratorRight->setDecoratee(cam->getCam()->cam);

    if (lView && PCDecoratorLeft) lView->setCamera(PCDecoratorLeft);
    if (rView && PCDecoratorRight) rView->setCamera(PCDecoratorRight);

    if (renderingL) renderingL->setCamera(cam->getCam());
    if (renderingR) renderingR->setCamera(cam->getCam());
    if (renderingL && PCDecoratorLeft) renderingL->setCamera( OSGCamera::create(PCDecoratorLeft) );
    if (renderingR && PCDecoratorRight) renderingR->setCamera( OSGCamera::create(PCDecoratorRight) );
}

void VRView::setBackground(BackgroundRecPtr bg) { background = bg; update(); }
void VRView::setWindow(WindowRecPtr win) { window = win; update(); }

void VRView::setOffset(Vec3d o) { offset = o; update(); }

void VRView::setWindow() {
    if (window == 0) return;
    if (lView) window->addPort(lView);
    if (rView) window->addPort(rView);
}

void VRView::setStereo(bool b) { stereo = b; update(); }
void VRView::setActiveStereo(bool b) { active_stereo = b; update(); }

void VRView::setStereoEyeSeparation(float v) {
    eyeSeparation = v;
    if (PCDecoratorLeft) PCDecoratorLeft->setEyeSeparation(v);
    if (PCDecoratorRight) PCDecoratorRight->setEyeSeparation(v);
}

void VRView::swapEyes(bool b) {
    eyeinverted = b;
    if (PCDecoratorLeft) PCDecoratorLeft->setLeftEye(!b);
    if (PCDecoratorRight) PCDecoratorRight->setLeftEye(b);
    renderingL->setEye( b ? VRRenderStudio::RIGHT : VRRenderStudio::LEFT );
    renderingR->setEye( b ? VRRenderStudio::LEFT : VRRenderStudio::RIGHT );
}

bool VRView::eyesInverted() { return eyeinverted; }
bool VRView::activeStereo() { return active_stereo; }

void VRView::update() {
    setViewports();
    setDecorators();
    setCam();
    setRoot();
    setUser();
    setWindow();
    setBG();
    swapEyes(eyeinverted);
    setStereoEyeSeparation(eyeSeparation);
    setMaterial();
}

void VRView::reset() {
    cam = 0;
    view_root = 0;
    if (renderingL) renderingL->reset();
    if (renderingR) renderingR->reset();
    update();
}

void VRView::setFotoMode(bool b) {
    if (!stereo) return;
    if (b) {
        if (PCDecoratorLeft) PCDecoratorLeft->setEyeSeparation(0);
        if (PCDecoratorRight) PCDecoratorRight->setEyeSeparation(0);
    } else update();
}

VRTexturePtr VRView::grab() {
    return takeSnapshot();

    /*if (grabfg == 0) {
        grabfg = GrabForeground::create();
        ImageRecPtr img = Image::create();
        grabfg->setImage(img);
        grabfg->setActive(false);
        if (lView) lView->editMFForegrounds()->push_back(grabfg);
    }


    if (lView) {
        grabfg->setActive(true);
        OSG::commitChanges();

        //window->render( VRSetup::getCurrent()->getRenderAction() );
        VRSetup::getCurrent()->updateWindows();
        VRGuiManager::get()->updateGtk(); // TODO: rendering produces just opengl error 500

        img = grabfg->getImage();
        if (img->getData()) img->write("bla.png");
        cout << "GRAB " << img->getData() << endl;
        grabfg->setActive(false);
    }
    return img;*/
}

void VRView::save(xmlpp::Element* node) {
    node->set_attribute("stereo", toString(stereo).c_str());
    node->set_attribute("active_stereo", toString(active_stereo).c_str());
    node->set_attribute("projection", toString(projection).c_str());
    node->set_attribute("eye_inverted", toString(eyeinverted).c_str());
    node->set_attribute("eye_separation", toString(eyeSeparation).c_str());
    node->set_attribute("position", toString(position).c_str());
    node->set_attribute("center", toString(proj_center).c_str());
    node->set_attribute("normal", toString(proj_normal).c_str());
    node->set_attribute("user_pos", toString(proj_user).c_str());
    node->set_attribute("up", toString(proj_up).c_str());
    node->set_attribute("size", toString(proj_size).c_str());
    node->set_attribute("shear", toString(proj_shear).c_str());
    node->set_attribute("warp", toString(proj_warp).c_str());
    node->set_attribute("vsize", toString(window_size).c_str());
    node->set_attribute("mirror", toString(mirror).c_str());
    node->set_attribute("mirrorPos", toString(mirrorPos).c_str());
    node->set_attribute("mirrorNorm", toString(mirrorNorm).c_str());
    if (user) node->set_attribute("user", user->getName());
    else node->set_attribute("user", user_name);
}

void VRView::load(xmlpp::Element* node) {
    stereo = toValue<bool>(node->get_attribute("stereo")->get_value());
    active_stereo = toValue<bool>(node->get_attribute("active_stereo")->get_value());
    projection = toValue<bool>(node->get_attribute("projection")->get_value());
    eyeinverted = toValue<bool>(node->get_attribute("eye_inverted")->get_value());
    eyeSeparation = toFloat(node->get_attribute("eye_separation")->get_value());
    position = toValue<Vec4d>(node->get_attribute("position")->get_value());
    proj_center = toValue<Vec3d>(node->get_attribute("center")->get_value());
    proj_normal = toValue<Vec3d>(node->get_attribute("normal")->get_value());
    if (node->get_attribute("user_pos")) proj_user = toValue<Vec3d>(node->get_attribute("user_pos")->get_value());
    proj_up = toValue<Vec3d>(node->get_attribute("up")->get_value());
    proj_size = toValue<Vec2d>(node->get_attribute("size")->get_value());
    if (node->get_attribute("shear")) proj_shear = toValue<Vec2d>(node->get_attribute("shear")->get_value());
    if (node->get_attribute("warp")) proj_warp = toValue<Vec2d>(node->get_attribute("warp")->get_value());
    if (node->get_attribute("vsize")) window_size = toValue<Vec2i>(node->get_attribute("vsize")->get_value());
    if (node->get_attribute("mirror")) mirror = toValue<bool>(node->get_attribute("mirror")->get_value());
    if (node->get_attribute("mirrorPos")) mirrorPos = toValue<Vec3d>(node->get_attribute("mirrorPos")->get_value());
    if (node->get_attribute("mirrorNorm")) mirrorNorm = toValue<Vec3d>(node->get_attribute("mirrorNorm")->get_value());
    if (node->get_attribute("user")) {
        user_name = node->get_attribute("user")->get_value();
        user = VRSetup::getCurrent()->getTracker(user_name);
    }

    if (ID == 0) {
        auto dev = VRSetup::getCurrent()->getDevice("multitouch"); // TODO, just a test, add mouse and multitouch to views!
        if (auto mt = dynamic_pointer_cast<VRMultiTouch>(dev)) mt->setViewport(ptr());
    }

    showStats(doStats);
    update();
}

VRTransformPtr VRView::getUser() { if (user) return user; else return dummy_user; }
VRCameraPtr VRView::getCamera() { return cam; }
ViewportRecPtr VRView::getViewport() { return lView; }
float VRView::getEyeSeparation() { return eyeSeparation; }
bool VRView::isStereo() { return stereo; }

void VRView::setProjection(bool b) { projection = b; update(); }
bool VRView::isProjection() { return projection; }

void VRView::setProjectionUp(Vec3d v) { proj_up = v; update(); }
Vec3d VRView::getProjectionUp() { return proj_up; }
void VRView::setProjectionNormal(Vec3d v) { proj_normal = v; update(); }
Vec3d VRView::getProjectionNormal() { return proj_normal; }
void VRView::setProjectionCenter(Vec3d v) { proj_center = v; update(); }
Vec3d VRView::getProjectionCenter() { return proj_center; }
void VRView::setProjectionSize(Vec2d v) { proj_size = v; update(); }
Vec2d VRView::getProjectionSize() { return proj_size; }
void VRView::setProjectionShear(Vec2d v) { proj_shear = v; update(); }
Vec2d VRView::getProjectionShear() { return proj_shear; }
void VRView::setProjectionWarp(Vec2d v) { proj_warp = v; update(); }
Vec2d VRView::getProjectionWarp() { return proj_warp; }

void VRView::setProjectionUser(Vec3d v) { proj_user = v; update(); }
Vec3d VRView::getProjectionUser() { return proj_user; }

OSG_END_NAMESPACE;
