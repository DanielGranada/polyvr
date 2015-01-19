#ifndef VRPyPath_H_INCLUDED
#define VRPyPath_H_INCLUDED

#include "core/math/path.h"
#include "core/scripting/VRPyBase.h"

struct VRPyPath : VRPyBaseT<OSG::path> {
    static PyMethodDef methods[];

    static PyObject* set(VRPyPath* self, PyObject *args);
    static PyObject* setStartPoint(VRPyPath* self, PyObject *args);
    static PyObject* setEndPoint(VRPyPath* self, PyObject *args);
    static PyObject* compute(VRPyPath* self, PyObject* args);
    static PyObject* setObjectDirection(VRPyPath* self, PyObject* args);
    static PyObject* getStartPoint(VRPyPath* self);
    static PyObject* getEndPoint(VRPyPath* self);
    static PyObject* update(VRPyPath* self);
};

#endif // VRPyPath_H_INCLUDED
