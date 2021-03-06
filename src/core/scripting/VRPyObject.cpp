#include "VRPyObject.h"
#include "VRPyBaseT.h"
#include "VRPyTypeCaster.h"
#include "core/objects/object/VRObject.h"
#include "core/objects/OSGObject.h"
#include "addons/Semantics/VRSemanticsFwd.h"

#include "VRPyBaseFactory.h"
#include "VRPyBoundingbox.h"

#include <OpenSG/OSGNode.h>

using namespace OSG;

template<> bool toValue(PyObject* o, VRObjectPtr& v) { if (!VRPyObject::check(o)) return 0; v = ((VRPyObject*)o)->objPtr; return 1; }

template<> PyTypeObject VRPyBaseT<OSG::VRObject>::type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "VR.Object",             /*tp_name*/
    sizeof(VRPyObject),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    VRPyObject::compare,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    VRPyObject::hash,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "VRObject binding",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    VRPyObject::methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)init,      /* tp_init */
    0,                         /* tp_alloc */
    New_VRObjects_ptr,                 /* tp_new */
};

PyMethodDef VRPyObject::methods[] = {
    {"getName", (PyCFunction)VRPyObject::getName, METH_NOARGS, "Return the object name" },
    {"getBaseName", (PyCFunction)VRPyObject::getBaseName, METH_NOARGS, "Return the object base name" },
    {"setName", (PyCFunction)VRPyObject::setName, METH_VARARGS, "Set the object name" },
    {"addChild", PyWrapOpt(Object, addChild, "Add object as child", "1|-1", void, VRObjectPtr, bool, int ) },
    {"switchParent", PyWrapOpt(Object, switchParent, "Switch object to other parent object", "-1", void, VRObjectPtr, int) },
    {"destroy", PyWrap(Object, destroy, "Destroy object", void) },
    {"hide", PyWrap(Object, hide, "Hide object", void) },
    {"show", PyWrap(Object, show, "Show object", void) },
    {"isVisible", PyWrap(Object, isVisible, "Return if object is visible", bool) },
    {"setVisible", PyWrap(Object, setVisible, "Set the visibility of the object", void, bool) },
    {"getType", PyWrap(Object, getType, "Return the object type string (such as \"Geometry\")", string) },
    {"getID", PyWrap(Object, getID, "Return the object internal ID", int) },
    {"duplicate", PyWrapOpt(Object, duplicate, "Duplicate object including subtree", "0", VRObjectPtr, bool) },
    {"getChild", PyWrap(Object, getChild, "Return child object with index i", VRObjectPtr, int) },
    {"getChildren", PyWrapOpt(Object, getChildren, "Return the list of children objects, bool recursive, string type-filter", "0||0", vector<VRObjectPtr>, bool, string, bool) },
    {"getParent", PyWrap(Object, getParent, "Return parent object", VRObjectPtr) },
    {"find", PyWrap(Object, find, "Find node with given name in scene graph below this node - obj find(str)", VRObjectPtr, string) },
    {"findAll", PyWrapOpt(Object, findAll, "Find nodes with given base name (str) in scene graph below this node", " ", vector<VRObjectPtr>, string, vector<VRObjectPtr>) },
    {"isPickable", PyWrap(Object, isPickable, "Return if the object is pickable", bool) },
    {"setPickable", PyWrap(Object, setPickable, "Set if the object is pickable - setPickable(int pickable)\n   pickable can be 0 or 1 to disable or enable picking, as well as -1 to block picking even if an ancestor is pickable", void, int) },
    //{"printOSG", PyWrap(Object, printOSGTree, "Print the OSG structure to console", void) },
    {"flattenHiarchy", PyWrap(Object, flattenHiarchy, "Flatten the scene graph hiarchy", void) },
    {"addTag", PyWrap(Object, addTag, "Add a tag to the object - addTag( str tag )", void, string) },
    {"hasTag", PyWrap(Object, hasTag, "Check if the object has a tag - bool hasTag( str tag )", bool, string) },
    {"remTag", PyWrap(Object, remTag, "Remove a tag from the object - remTag( str tag )", void, string) },
    {"getTags", PyWrap(Object, getTags, "Return all tags - [str] getTags()", vector<string>) },
    {"hasAncestorWithTag", PyWrap(Object, hasAncestorWithTag, "Check if the object or an ancestor has a tag - obj hasAncestorWithTag( str tag )", VRObjectPtr, string) },
    {"getChildrenWithTag", PyWrap(Object, getChildrenWithTag, "Get all children which have the tag - [objs] getChildrenWithTag( str tag )", vector<VRObjectPtr>, string) },
    {"setVolumeCheck", PyWrapOpt(Object, setVolumeCheck, "Enables or disabled the dynamic volume computation of that node - setVolumeCheck( bool )", "0", void, bool, bool) },
    {"setTravMask", PyWrap(Object, setTravMask, "Set the traversal mask of the object - setTravMask( int mask )", void, int) },
    {"setPersistency", (PyCFunction)VRPyObject::setPersistency, METH_VARARGS, "Set the persistency level - setPersistency( int persistency | bool recursive )\n   0: not persistent\n   1: persistent hiarchy\n   2: transformation\n   3: geometry\n   4: fully persistent" },
    {"getPersistency", (PyCFunction)VRPyObject::getPersistency, METH_NOARGS, "Get the persistency level - getPersistency()" },
    {"addLink", PyWrap(Object, addLink, "Link subtree - addLink( object )", void, VRObjectPtr) },
    {"remLink", PyWrap(Object, remLink, "Unlink subtree - remLink( object )", void, VRObjectPtr) },
    {"setEntity", PyWrap(Object, setEntity, "Set entity", void, VREntityPtr) },
    {"getEntity", PyWrap(Object, getEntity, "Get entity", VREntityPtr) },
    {"clearChildren", PyWrap(Object, clearChildren, "Remove all children - clearChildren()", void) },
    {"getChildIndex", PyWrap(Object, getChildIndex, "Return the child index of this object - int getChildIndex()", int) },
    {"getBoundingbox", PyWrap(Object, getBoundingbox, "get Boundingbox", BoundingboxPtr) },
    {"setVolume", PyCastWrap(Object, setVolume, "Set the scenegraph volume to boundingbox", void, Boundingbox) },
    {NULL}  /* Sentinel */
};

PyObject* VRPyObject::setPersistency(VRPyObject* self, PyObject* args) {
    if (self->objPtr == 0) { PyErr_SetString(err, "VRPyObject::setPersistency - C Object is invalid"); return NULL; }
    int i = 0;
    int b = 0;
    if (!PyArg_ParseTuple(args, "i|i", &i, &b)) return NULL;
    self->objPtr->setPersistency( i );
    if (b) for (auto c : self->objPtr->getChildren(true)) c->setPersistency(i);
    Py_RETURN_TRUE;
}

PyObject* VRPyObject::getPersistency(VRPyObject* self) {
    if (self->objPtr == 0) { PyErr_SetString(err, "VRPyObject::getPersistency - C Object is invalid"); return NULL; }
    return PyInt_FromLong( self->objPtr->getPersistency() );
}

int VRPyObject::compare(PyObject* p1, PyObject* p2) {
    if (Py_TYPE(p1) != Py_TYPE(p2)) return -1;
    VRPyBaseT* o1 = (VRPyBaseT*)p1;
    VRPyBaseT* o2 = (VRPyBaseT*)p2;
    return (o1->objPtr == o2->objPtr) ? 0 : -1;
}

long VRPyObject::hash(PyObject* p) {
    VRPyBaseT* o = (VRPyBaseT*)p;
    return (long)o->objPtr.get();
}

PyObject* VRPyObject::getName(VRPyObject* self) {
    if (self->objPtr == 0) { PyErr_SetString(err, "C Object is invalid"); return NULL; }
    return PyString_FromString(self->objPtr->getName().c_str());
}

PyObject* VRPyObject::getBaseName(VRPyObject* self) {
    if (self->objPtr == 0) { PyErr_SetString(err, "C Object is invalid"); return NULL; }
    return PyString_FromString(self->objPtr->getBaseName().c_str());
}

PyObject* VRPyObject::setName(VRPyObject* self, PyObject* args) {
    if (self->objPtr == 0) { PyErr_SetString(err, "C Object is invalid"); return NULL; }
    string name = parseString(args);
    self->objPtr->setName(name);
    Py_RETURN_TRUE;
}

