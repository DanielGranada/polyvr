#ifndef VRVIEW_H_INCLUDED
#define VRVIEW_H_INCLUDED

#include <OpenSG/OSGProjectionCameraDecorator.h>
#include <OpenSG/OSGSolidBackground.h>
#include <OpenSG/OSGStereoBufferViewport.h>
#include <OpenSG/OSGSimpleStatisticsForeground.h>
#include <OpenSG/OSGGrabForeground.h>
#include <OpenSG/OSGImageForeground.h>
#include "core/objects/VRObjectFwd.h"
#include "core/setup/VRSetupFwd.h"
#include "core/scene/rendering/VRRenderManager.h"

namespace xmlpp{ class Element; }

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRLight;
class VRDefShading;
class VRSSAO;
class VRHMDDistortion;
class VRCamera;

class VRView : public std::enable_shared_from_this<VRView> {
    private:
        int ID;
        string name;

        bool active_stereo = false;
        bool stereo = false;
        bool projection = false;
        bool doStats = false;

        NodeMTRecPtr viewGeo;
        VRMaterialPtr viewGeoMat;

        Vec4d position = Vec4d(0,0,1,1);
        Vec2i window_size = Vec2i(300,300);

        float eyeSeparation = 0.06;
        bool eyeinverted = false;

        //stereo decorator
        ProjectionCameraDecoratorRecPtr PCDecoratorLeft;
        ProjectionCameraDecoratorRecPtr PCDecoratorRight;

        WindowRecPtr window;
        VRRenderStudioPtr renderingL;
        VRRenderStudioPtr renderingR;

        //ShearedStereoCameraDecoratorPtr SSCDecoratorLeft;
        //ShearedStereoCameraDecoratorPtr SSCDecoratorRight;

        Vec3d offset;

        Vec3d proj_user = Vec3d(0,0,0);
        Vec3d proj_center = Vec3d(0,0,-1);
        Vec3d proj_up = Vec3d(0,1,0);
        Vec3d proj_normal = Vec3d(0,0,1);
        Vec2d proj_size = Vec2d(2,1);
        Vec2d proj_shear = Vec2d(0,0);
        Vec2d proj_warp = Vec2d(0,0);

        Pnt3f screenLowerLeft;
        Pnt3f screenLowerRight;
        Pnt3f screenUpperRight;
        Pnt3f screenUpperLeft;

        bool mirror = false;
        Vec3d mirrorPos = Vec3d(0,0,0);
        Vec3d mirrorNorm = Vec3d(0,0,1);

        //stereo viewports
        ViewportRecPtr lView;//used also for non stereo
        ViewportRecPtr rView;
        //active stereo
        StereoBufferViewportRecPtr lView_act;
        StereoBufferViewportRecPtr rView_act;

        //headtracking user
        VRObjectPtr view_root;
        VRTransformPtr real_root;
        VRTransformPtr user;
        VRTransformPtr dummy_user;
        string user_name;
        VRCameraPtr cam;

        BackgroundRecPtr background;
        SimpleStatisticsForegroundRecPtr stats = 0;
        GrabForegroundRecPtr grabfg = 0;

        void setMaterial();
        void setViewports();
        void setDecorators();
        void setCam();
        void setBG();
        void setUser();
        void setWindow();
        void setRoot();

    public:
        VRView(string name);
        ~VRView();

        static VRViewPtr create(string name);
        VRViewPtr ptr();

        int getID();
        void setID(int i);

        string getName();
        void setSize(Vec2i s);
        Vec2i getSize();

        void setRoot(VRObjectPtr root, VRTransformPtr _real);
        void setUser(VRTransformPtr u);
        void setCamera(VRCameraPtr c);
        void setBackground(BackgroundRecPtr bg);
        void setWindow(WindowRecPtr win);
        void setStereo(bool b);
        void setStereoEyeSeparation(float v);
        void setProjection(bool b);
        void setOffset(Vec3d);

        VRTransformPtr getUser();
        VRCameraPtr getCamera();
        bool isStereo();
        float getEyeSeparation();
        bool isProjection();

        void setProjectionUp(Vec3d v);
        Vec3d getProjectionUp();
        void setProjectionNormal(Vec3d v);
        Vec3d getProjectionNormal();
        void setProjectionCenter(Vec3d v);
        Vec3d getProjectionCenter();
        void setProjectionUser(Vec3d v);
        Vec3d getProjectionUser();
        void setProjectionSize(Vec2d v);
        Vec2d getProjectionSize();
        void setProjectionShear(Vec2d v);
        Vec2d getProjectionShear();
        void setProjectionWarp(Vec2d v);
        Vec2d getProjectionWarp();

        void showStats(bool b);
        void showViewGeo(bool b);

        Vec4d getPosition();
        void setPosition(Vec4d pos);

        ViewportRecPtr getViewport();
        VRRenderStudioPtr getRenderingL();
        VRRenderStudioPtr getRenderingR();

        void swapEyes(bool b);
        bool eyesInverted();
        void setActiveStereo(bool b);
        bool activeStereo();

        void setMirror(bool b);
        bool getMirror();
        void setMirrorPos(Vec3d p);
        Vec3d getMirrorPos();
        void setMirrorNorm(Vec3d n);
        Vec3d getMirrorNorm();

        void update();
        void reset();

        void setFotoMode(bool b);
        VRTexturePtr grab();

        void save(xmlpp::Element* node);
        void load(xmlpp::Element* node);
};

OSG_END_NAMESPACE;

#endif // VRVIEW_H_INCLUDED
