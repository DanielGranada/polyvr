#include "VRProcess.h"

#include "addons/Semantics/Reasoning/VROntology.h"
#include "addons/Semantics/Reasoning/VRReasoner.h"
#include "addons/Semantics/Reasoning/VRProperty.h"
#include "core/utils/VRStorage_template.h"
#include "core/objects/VRTransform.h"
#include <iostream>

using namespace OSG;

VRProcessNode::VRProcessNode() {;}

void VRProcessNode::update(graph_base::node& n, bool changed) { // callede when graph node changes
    if (widget && !widget->isDragged() && changed) widget->setFrom(n.box.center());

    if (widget && widget->isDragged()) {
        auto m = widget->getMatrixTo( widget->getDragParent() );
        n.box.setCenter( Vec3f(m[3]) );
    }
}

int VRProcessNode::getID() { return ID; }
string VRProcessNode::getLabel() { return label; }

VRProcess::VRProcess(string name) {
    setStorageType("Process");
    setPersistency(0);
    setNameSpace("Process");
    setName(name);

    storeObj("Ontology", ontology);
}

VRProcessPtr VRProcess::create(string name) { return VRProcessPtr( new VRProcess(name) ); }
VRProcessPtr VRProcess::ptr() { return shared_from_this(); }

void VRProcess::open(string path) {
    ontology = VROntology::create(getBaseName());
    ontology->open(path);
    update();
}

void VRProcess::setOntology(VROntologyPtr o) { ontology = o; update(); }

VRProcess::DiagramPtr VRProcess::getInteractionDiagram() { return interactionDiagram; }
VRProcess::DiagramPtr VRProcess::getBehaviorDiagram(int subject) { return behaviorDiagrams.count(subject) ? behaviorDiagrams[subject] : 0; }

vector<VRProcessNode> VRProcess::getSubjects() {
    vector<VRProcessNode> res;
    for (int i=0; i<interactionDiagram->size(); i++) {
        auto& e = interactionDiagram->getElement(i);
        if (e.type == SUBJECT) res.push_back(e);
    }
    return res;
}

void VRProcess::update() {
    if (!ontology) return;

    VRReasonerPtr reasoner = VRReasoner::create();
    reasoner->setVerbose(true,  false); //
    auto query = [&](string q) { return reasoner->process(q, ontology); };

    map<string, int> nodes;
    auto addDiagNode = [&](DiagramPtr diag, string label, PROCESS_WIDGET type) {
        auto n = VRProcessNode();
        n.label = label;
        n.type = type;
        int i = diag->addNode(n);
        diag->getElement(i).ID = i;
        return i;
    };

    auto connect = [&](DiagramPtr diag, int i, string parent, graph_base::CONNECTION mode) {
        if (nodes.count(parent)) {
            switch (mode) {
                case graph_base::HIERARCHY: diag->connect(nodes[parent], i, mode); break;
                case graph_base::DEPENDENCY: diag->connect(i, nodes[parent], mode); break;
            }
        } else cout << "VRProcess::connect " << parent << " not in nodes\n";
    };

    /** get interaction diagram **/

    auto layers = query("q(x):Layer(x)");
    if (layers.size() == 0) return;
    auto layer = layers[0]; // only use first layer
    interactionDiagram = DiagramPtr( new Diagram() );

    string q_subjects = "q(x):ActiveProcessComponent(x);Layer("+layer->getName()+");has("+layer->getName()+",x)";
    for ( auto subject : query(q_subjects) ) {
        string label;
        if (auto l = subject->get("hasModelComponentLable") ) label = l->value;
        int nID = addDiagNode(interactionDiagram, label, SUBJECT);
        if (auto ID = subject->get("hasModelComponentID") ) nodes[ID->value] = nID;
    }

    map<string, map<string, vector<VREntityPtr>>> messages;
    string q_messages = "q(x):MessageExchange(x);Layer("+layer->getName()+");has("+layer->getName()+",x)";
    for ( auto message : query(q_messages) ) {
        string sender;
        string receiver;
        if (auto s = message->get("sender") ) sender = s->value;
        if (auto r = message->get("receiver") ) receiver = r->value;
        messages[sender][receiver].push_back(message);
    }

    for ( auto sender : messages ) {
        for (auto receiver : sender.second) {
            string label = "Msg:";
            for (auto message : receiver.second) {
                string q_message = "q(x):MessageSpec(x);MessageExchange("+message->getName()+");is(x,"+message->getName()+".hasMessageType)";
                auto msgs = query(q_message);
                if (msgs.size())
                    if (auto l = msgs[0]->get("hasModelComponentLable") ) label += "\n - " + l->value;
            }

            int nID = addDiagNode(interactionDiagram, label, MESSAGE);
            connect(interactionDiagram, nID, sender.first, graph_base::HIERARCHY);
            connect(interactionDiagram, nID, receiver.first, graph_base::DEPENDENCY);
        }
    }

    //return;
    /** get behavior diagrams **/

    for (auto behavior : query("q(x):Behavior(x)")) {
        auto behaviorDiagram = DiagramPtr( new Diagram() );
        string q_Subject = "q(x):ActiveProcessComponent(x);Behavior("+behavior->getName()+");has(x,"+behavior->getName()+")";
        auto subjects = query(q_Subject);
        if (subjects.size() == 0) continue;
        auto subject = subjects[0];
        auto ID = subject->get("hasModelComponentID");
        int sID = nodes[ID->value];
        behaviorDiagrams[sID] = behaviorDiagram;

        string q_States = "q(x):State(x);Behavior("+behavior->getName()+");has("+behavior->getName()+",x)";
        for (auto state : query(q_States)) {
            string label;
            if (auto l = state->get("hasModelComponentLable") ) label = l->value;
            int nID = addDiagNode(behaviorDiagram, label, SUBJECT);
            if (auto ID = state->get("hasModelComponentID") ) nodes[ID->value] = nID;
        }

        map<string, map<string, vector<VREntityPtr>>> edges;
        string q_Edges = "q(x):TransitionEdge(x);Behavior("+behavior->getName()+");has("+behavior->getName()+",x)";
        for (auto edge : query(q_Edges)) {
            string source;
            string target;
            if (auto s = edge->get("hasSourceState") ) source = s->value;
            if (auto r = edge->get("hasTargetState") ) target = r->value;
            edges[source][target].push_back(edge);
        }

        for ( auto source : edges ) {
            cout << "source " << source.first << endl;
            for (auto target : source.second) {
                cout << "target " << target.first << endl;
                string label = "Msg:";
                /*for (auto edge : target.second) {
                    string q_message = "q(x):MessageSpec(x);MessageExchange("+edge->getName()+");is(x,"+edge->getName()+".hasMessageType)";
                    auto msgs = query(q_message);
                    if (msgs.size())
                        if (auto l = msgs[0]->get("hasModelComponentLable") ) label += "\n - " + l->value;
                }*/

                int nID = addDiagNode(behaviorDiagram, label, MESSAGE);
                connect(behaviorDiagram, nID, source.first, graph_base::HIERARCHY);
                connect(behaviorDiagram, nID, target.first, graph_base::DEPENDENCY);
                //connect(behaviorDiagram, nodes[source.first], target.first, graph_base::DEPENDENCY);
            }
        }
    }
}




