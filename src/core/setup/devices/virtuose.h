#ifndef VIRTUOSE_H_INCLUDED
#define VIRTUOSE_H_INCLUDED
#include <virtuose/virtuoseAPI.h>

#include <OpenSG/OSGVector.h>
#include <OpenSG/OSGMatrix.h>

#include <list>
#include "core/objects/VRTransform.h"
#include "core/objects/geometry/VRPhysics.h"


OSG_BEGIN_NAMESPACE;
using namespace std;

class virtuose {
    private:
        void* vc = 0; // virtuose context
        bool isAttached = false;
        VRTransform* attached = 0;
        VirtCommandType commandType  = COMMAND_TYPE_NONE;
        float gripperPosition;
        float gripperSpeed;

    public:
        virtuose();
        ~virtuose();

        bool connected();
        void connect(string IP);
        void disconnect();

        void setSimulationScales(float translation, float forces);
        void applyForce(Vec3f force, Vec3f torque);
        Matrix getPose();

        //connect a physicalized Object to this virtuose and push it in the same direction the virtuose moves . apply forces( which affect the object )on the haptic.
        void updateVirtMech();
        Vec3f getForce();
        void attachTransform(VRTransform* trans);
        void detachTransform();
};

OSG_END_NAMESPACE;

#endif // VIRTUOSE_H_INCLUDED
