#pragma once

#include "core/objects/geometry/VRGeometry.h"
#include "VRLeap.h"

OSG_BEGIN_NAMESPACE ;

class VRHandGeo : public VRGeometry {

public:

    static VRHandGeoPtr create(string name = "HandGeo");

    void update(VRLeapFramePtr frame);

    void setLeft();

    void setRight();

    void updateChange();

private:

    VRHandGeo(string name);

    boost::mutex mutex;
    bool isLeft{false};
    bool initialized{false};
    bool visible{false};

    vector<vector<VRGeometryPtr>> bones;
    vector<VRGeometryPtr> directions;
    VRGeometryPtr pinch;
    VRLeapFrame::HandPtr handData;
};

OSG_END_NAMESPACE;
