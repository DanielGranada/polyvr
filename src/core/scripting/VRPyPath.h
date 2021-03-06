#ifndef VRPyPath_H_INCLUDED
#define VRPyPath_H_INCLUDED

#include "core/math/path.h"
#include "core/scripting/VRPyBase.h"

struct VRPyPath : VRPyBaseT<OSG::path> {
    static PyMethodDef methods[];

    static PyObject* set(VRPyPath* self, PyObject *args);
    static PyObject* compute(VRPyPath* self, PyObject* args);
    static PyObject* invert(VRPyPath* self);
    static PyObject* close(VRPyPath* self);
    static PyObject* update(VRPyPath* self);
    static PyObject* addPoint(VRPyPath* self, PyObject *args);
    static PyObject* getPositions(VRPyPath* self);
    static PyObject* getDirections(VRPyPath* self);
    static PyObject* getUpVectors(VRPyPath* self);
    static PyObject* getColors(VRPyPath* self);
    static PyObject* getPoints(VRPyPath* self);
    static PyObject* getSize(VRPyPath* self);
    static PyObject* getLength(VRPyPath* self, PyObject *args);
    static PyObject* getPose(VRPyPath* self, PyObject *args);
    static PyObject* getDistance(VRPyPath* self, PyObject *args);
    static PyObject* getDistanceToHull(VRPyPath* self, PyObject *args);
    static PyObject* getClosestPoint(VRPyPath* self, PyObject *args);
    static PyObject* approximate(VRPyPath* self, PyObject *args);
    static PyObject* isStraight(VRPyPath* self, PyObject *args);
    static PyObject* isCurve(VRPyPath* self, PyObject *args);
    static PyObject* isSinuous(VRPyPath* self, PyObject *args);
};

#endif // VRPyPath_H_INCLUDED
