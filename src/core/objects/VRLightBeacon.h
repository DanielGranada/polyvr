#ifndef VRLIGHTBEACON_H_INCLUDED
#define VRLIGHTBEACON_H_INCLUDED

#include "VRTransform.h"

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRLight;

class VRLightBeacon : public VRTransform {
    protected:
        VRLightWeakPtr light;
        string light_name;
        OSGObjectPtr lightGeo;

    public:
        VRLightBeacon(string name);
        ~VRLightBeacon();

        static VRLightBeaconPtr create(string name = "None");
        VRLightBeaconPtr ptr();

        VRLightWeakPtr getLight();
        void setLight(VRLightPtr l);

        void showLightGeo(bool b);

        static vector<VRLightBeaconPtr>& getAll();
};

OSG_END_NAMESPACE;

#endif // VRLIGHTBEACON_H_INCLUDED
