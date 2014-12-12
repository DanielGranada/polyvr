#ifndef VRSCRIPTMANAGER_H_INCLUDED
#define VRSCRIPTMANAGER_H_INCLUDED

#include <OpenSG/OSGConfig.h>
#include <map>
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <string>
#include "core/setup/devices/VRSignal.h"
#include "core/utils/VRStorage.h"
#include "VRPyBase.h"

namespace xmlpp{ class Element; }

OSG_BEGIN_NAMESPACE;
using namespace std;

class VRScript;

class VRScriptManager : public VRStorage, public VRPyBase {
    private:
        PyObject* pGlobal;
        PyObject* pLocal;
        PyObject* pModBase;
        PyObject* pModVR;
        map<string, VRScript*> scripts;
        map<string, VRSignal*> triggers;

        void test();

        void initPyModules();

    protected:
        void update();

    public:
        VRScriptManager();
        ~VRScriptManager();

        VRScript* newScript(string name, string function);
        void remScript(string name);

        void disableAllScripts();

        void updateScript(string name, string core, bool compile = true);
        VRScript* changeScriptName(string name, string new_name);

        void triggerScript(string name);

        VRScript* getScript(string name);
        map<string, VRScript*> getScripts();

        //void saveScripts(xmlpp::Element* e);
        //void loadScripts(xmlpp::Element* e);

        vector<string> getPyVRTypes();
        vector<string> getPyVRMethods(string type);
        string getPyVRMethodDoc(string type, string method);

        // Python Methods
		static PyObject* exit(VRScriptManager* self);
		static PyObject* loadGeometry(VRScriptManager* self, PyObject *args);
		static PyObject* stackCall(VRScriptManager* self, PyObject *args);
		static PyObject* openFileDialog(VRScriptManager* self, PyObject *args);
		static PyObject* updateGui(VRScriptManager* self);
};

OSG_END_NAMESPACE

#endif // VRSCRIPTMANAGER_H_INCLUDED
