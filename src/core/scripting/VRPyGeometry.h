#ifndef VRPYGEOMETRY_H_INCLUDED
#define VRPYGEOMETRY_H_INCLUDED

#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <structmember.h>
#include "VRPyTransform.h"
#include "core/objects/geometry/VRGeometry.h"

struct VRPyGeometry : VRPyBaseT<OSG::VRGeometry> {

    static PyMemberDef members[];
    static PyMethodDef methods[];

    // wrapped methods
    static PyObject* setType(VRPyGeometry* self, PyObject *args);
    static PyObject* setTypes(VRPyGeometry* self, PyObject *args);
    static PyObject* setPositions(VRPyGeometry* self, PyObject *args);
    static PyObject* setNormals(VRPyGeometry* self, PyObject *args);
    static PyObject* setColors(VRPyGeometry* self, PyObject *args);
    static PyObject* setLengths(VRPyGeometry* self, PyObject *args);
    static PyObject* setIndices(VRPyGeometry* self, PyObject *args);
    static PyObject* setTexCoords(VRPyGeometry* self, PyObject *args);
    static PyObject* setTexture(VRPyGeometry* self, PyObject *args);
    static PyObject* setVideo(VRPyGeometry* self, PyObject *args);
    static PyObject* playVideo(VRPyGeometry* self, PyObject *args);
    static PyObject* setMaterial(VRPyGeometry* self, PyObject *args);
    static PyObject* getType(VRPyGeometry* self);
    static PyObject* getTypes(VRPyGeometry* self);
    static PyObject* getPositions(VRPyGeometry* self);
    static PyObject* getNormals(VRPyGeometry* self);
    static PyObject* getColors(VRPyGeometry* self);
    static PyObject* getIndices(VRPyGeometry* self);
    static PyObject* getTexCoords(VRPyGeometry* self);
    static PyObject* getTexture(VRPyGeometry* self);
    static PyObject* getMaterial(VRPyGeometry* self);
    static PyObject* duplicate(VRPyGeometry* self);
    static PyObject* setLit(VRPyGeometry* self, PyObject *args);
    static PyObject* setPrimitive(VRPyGeometry* self, PyObject *args);
    static PyObject* decimate(VRPyGeometry* self, PyObject *args);
    static PyObject* setRandomColors(VRPyGeometry* self);
    static PyObject* makeUnique(VRPyGeometry* self);
    static PyObject* removeDoubles(VRPyGeometry* self, PyObject *args);
};

#endif // VRPYGEOMETRY_H_INCLUDED