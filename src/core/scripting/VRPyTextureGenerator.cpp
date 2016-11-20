#include "VRPyTextureGenerator.h"
#include "VRPyGeometry.h"
#include "VRPyDevice.h"
#include "VRPyBaseT.h"
#include "VRPyImage.h"
#include "VRPyPath.h"

#include "core/math/path.h"

using namespace OSG;

simpleVRPyType(TextureGenerator, New);

PyMethodDef VRPyTextureGenerator::methods[] = {
    {"add", (PyCFunction)VRPyTextureGenerator::add, METH_VARARGS, "Add a layer - add(str type, float amount, [r,g,b], [r,g,b])\n\ttype can be: 'Perlin', 'Bricks'"
                                                                    "\n\t add(str type, float amount, [r,g,b,a], [r,g,b,a])" },
    {"drawFill", (PyCFunction)VRPyTextureGenerator::drawFill, METH_VARARGS, "Set the size - drawFill([r,g,b,a])\n   Fill whole texture" },
    {"drawLine", (PyCFunction)VRPyTextureGenerator::drawLine, METH_VARARGS, "Set the size - drawLine([x1,y1,z1], [x2,y2,z2], [r,g,b,a], flt width)\n   Add a line, coordinates go from 0 to 1" },
    {"drawPath", (PyCFunction)VRPyTextureGenerator::drawPath, METH_VARARGS, "Set the size - drawPath(path, [r,g,b,a], flt width)\n   Add a path, use normalized coordinates from 0 to 1" },
    {"setSize", (PyCFunction)VRPyTextureGenerator::setSize, METH_VARARGS, "Set the size - setSize([width, height, depth] | bool hasAlphaChannel)\n   set depth to 1 for 2D textures" },
    {"compose", (PyCFunction)VRPyTextureGenerator::compose, METH_VARARGS, "Bake the layers into an image - img compose( int seed )" },
    {"readSharedMemory", (PyCFunction)VRPyTextureGenerator::readSharedMemory, METH_VARARGS, "Read an image from shared memory - img readSharedMemory( string segment, string data )" },
    {NULL}  /* Sentinel */
};

PyObject* VRPyTextureGenerator::compose(VRPyTextureGenerator* self, PyObject* args) {
    return VRPyImage::fromSharedPtr( self->obj->compose( parseInt(args) ) );
}

PyObject* VRPyTextureGenerator::readSharedMemory(VRPyTextureGenerator* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyTextureGenerator::add - Object is invalid"); return NULL; }
    const char *segment, *data;
    if (! PyArg_ParseTuple(args, "ss", (char*)&segment, (char*)&data)) return NULL;
    if (segment && data) return VRPyImage::fromSharedPtr( self->obj->readSharedMemory(segment, data) );
    Py_RETURN_NONE;
}

PyObject* VRPyTextureGenerator::drawFill(VRPyTextureGenerator* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyTextureGenerator::add - Object is invalid"); return NULL; }
    PyObject* c;
    if (! PyArg_ParseTuple(args, "O", &c)) return NULL;
    self->obj->drawFill(parseVec4fList(c));
    Py_RETURN_TRUE;
}

PyObject* VRPyTextureGenerator::drawLine(VRPyTextureGenerator* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyTextureGenerator::add - Object is invalid"); return NULL; }
    PyObject *p1, *p2, *c;
    float w;
    if (! PyArg_ParseTuple(args, "OOOf", &p1, &p2, &c, &w)) return NULL;
    self->obj->drawLine(parseVec3fList(p1), parseVec3fList(p2), parseVec4fList(c), w);
    Py_RETURN_TRUE;
}

PyObject* VRPyTextureGenerator::drawPath(VRPyTextureGenerator* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyTextureGenerator::add - Object is invalid"); return NULL; }
    VRPyPath* p;
    PyObject* c;
    float w;
    if (! PyArg_ParseTuple(args, "OOf", &p, &c, &w)) return NULL;
    OSG::pathPtr pp = OSG::pathPtr( new OSG::path() );
    *pp = *(p->obj);
    self->obj->drawPath(pp, parseVec4fList(c), w);
    Py_RETURN_TRUE;
}

PyObject* VRPyTextureGenerator::add(VRPyTextureGenerator* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyTextureGenerator::add - Object is invalid"); return NULL; }
    PyObject *type, *c1, *c2;
    float amount;
    if (! PyArg_ParseTuple(args, "OfOO", &type, &amount, &c1, &c2)) return NULL;
    if (pySize(c1) == 3) self->obj->add(PyString_AsString(type), amount, parseVec3fList(c1), parseVec3fList(c2));
    if (pySize(c1) == 4) self->obj->add(PyString_AsString(type), amount, parseVec4fList(c1), parseVec4fList(c2));
    Py_RETURN_TRUE;
}

PyObject* VRPyTextureGenerator::setSize(VRPyTextureGenerator* self, PyObject* args) {
    if (self->obj == 0) { PyErr_SetString(err, "VRPyTextureGenerator::add - Object is invalid"); return NULL; }
    PyObject* s; int b = 0;
    if (! PyArg_ParseTuple(args, "O|i", &s, &b)) return NULL;
    self->obj->setSize( parseVec3iList(s), b );
    Py_RETURN_TRUE;
}
