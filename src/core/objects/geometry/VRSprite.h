#ifndef OSGPANEL_H_INCLUDED
#define OSGPANEL_H_INCLUDED

#include "VRGeometry.h"
#include <OpenSG/OSGSimpleTexturedMaterial.h>


OSG_BEGIN_NAMESPACE;
using namespace std;

class VRSprite : public VRGeometry {
    protected:

        SimpleTexturedMaterialRecPtr labelMat;

        string font;
        Color4f fontColor;
        float width;
        float height;

        string label;

    public:

        VRSprite (string name, bool alpha = true, float w = 0.5, float h = 0.5);

        void setSize(float w, float h);

        void setLabel(string l, float w = 100, float h = 100);

        void setTexture(string path);

        void setFont(string f);
        void setFontColor(Color4f c);

        //-------------------------- GET

        string getLabel();

};

OSG_END_NAMESPACE;

#endif // OSGPANEL_H_INCLUDED