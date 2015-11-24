#include "VROntology.h"
#include "VRReasoner.h"
#include "core/utils/toString.h"

/* no compiling?
    install the raptor2 ubuntu package for rdfxml parsing
    sudo apt-get install libraptor2-dev
*/

#include <raptor2/raptor2.h> // http://librdf.org/raptor/api/
#include <iostream>

VROntology::VROntology() {
    thing = new VRConcept("Thing");
}

VROntologyPtr VROntology::create() { return VROntologyPtr( new VROntology() ); }

VRConcept* VROntology::getConcept(string name, VRConcept* p) {
    if (p == 0) p = thing;
    if (p->name == name) return p;
    VRConcept* c = 0;
    for (auto ci : p->children) {
        c = getConcept(name, ci.second);
        if (c) return c;
    }
    return c;
}

VRConcept* VROntology::addConcept(string concept, string parent) {
    if (parent == "") return thing->append(concept);
    auto p = getConcept(parent);
    if (p == 0) { cout << "WARNING in VROntology::addConcept, " << parent << " not found while adding " << concept << "!\n"; return 0;  }
    return getConcept(parent)->append(concept);
}

string VROntology::answer(string question) {
    auto res = VRReasoner::get()->process(question, this);
    return "";//res.toString();
}

void VROntology::merge(VROntology* o) {
    for (auto c : o->thing->children)
        thing->append(c.second);
    for (auto c : o->rules)
        rules[c.first] = c.second;
}

vector<VROntologyRule*> VROntology::getRules() {
    vector<VROntologyRule*> res;
    for (auto r : rules) res.push_back(r.second);
    return res;
}

VROntologyRule* VROntology::addRule(string rule) {
    VROntologyRule* r = new VROntologyRule(rule);
    rules[r->ID] = r;
    return r;
}

VREntity* VROntology::addVectorInstance(string name, string concept, string x, string y, string z) {
    auto i = addInstance(name, concept);
    i->set("x", x);
    i->set("y", y);
    i->set("z", z);
    return i;
}

VREntity* VROntology::addInstance(string name, string concept) {
    auto c = getConcept(concept);
    auto i = new VREntity(name, c);
    instances[i->ID] = i;
    return i;
}

VREntity* VROntology::getInstance(string instance) {
    for (auto i : instances) if (i.second->name == instance) return i.second;
    return 0;
}

vector<VREntity*> VROntology::getInstances(string concept) {
    vector<VREntity*> res;
    for (auto i : instances) if (i.second->concept->is_a(concept)) res.push_back(i.second);
    return res;
}

struct RDFStatement {
    string graph;
    string object;
    string predicate;
    string subject;

    string toString(raptor_term* t) {
        if (t == 0) return "";
        switch(t->type) {
            case RAPTOR_TERM_TYPE_LITERAL:
                return string( (const char*)t->value.literal.string );
            case RAPTOR_TERM_TYPE_BLANK:
                return "BLANK";
            case RAPTOR_TERM_TYPE_UNKNOWN:
                return "UNKNOWN";
            case RAPTOR_TERM_TYPE_URI:
                auto uri = raptor_uri_as_string( t->value.uri );
                if (!uri) return "";
                string s( (const char*)uri );
                auto ss = splitString(s, '#');
                //raptor_free_memory(uri);
                return ss[ss.size()-1];
        }
        return "";
    }

    RDFStatement(raptor_statement* s) {
        graph = toString(s->graph);
        object = toString(s->object);
        predicate = toString(s->predicate);
        subject = toString(s->subject);
    }

    string toString() {
        return "Statement:\n graph: "+graph+"\n object: "+object+"\n predicate: "+predicate+"\n subject: "+subject;
    }
};

struct RDFdata {
    map<string, map<string, string> > objects;
};

void print_triple(void* data, raptor_statement* rs) {
    auto RDFSubjects = (RDFdata*)data;
    auto s = RDFStatement(rs);
    //cout << s.toString() << endl;
    RDFSubjects->objects[s.subject][s.object] = s.predicate;
}

void postProcessRDFSubjects(VROntology* onto, RDFdata& data) {
    map<string, map<string,string> > classes;
    map<string, map<string,string> > individuals;

    // add all classes
    for (auto& d : data.objects) {
        if (d.second.count("Class")) classes[d.first] = d.second;
        if (d.second.count("NamedIndividual")) individuals[d.first] = d.second;
    }

    map<string, string> s;
    for (auto& c : classes) {
        string subject = c.first;
        auto& objects = c.second;

        string parent = "";
        for (auto obj : objects) if (obj.second == "subClassOf") parent = obj.first;
        if (parent == "") {
            cout << "Add root concept " << subject << endl;
            onto->addConcept(subject);
            continue;
        }

        if (!onto->getConcept(parent)) {
            s[subject] = parent;
            continue;
        }

        cout << "Add concept " << subject << " as subconcept of " << parent << endl;
        onto->addConcept( subject, parent );
    }

    while (s.size()) {
        map<string, string> tmp;
        for (auto& c : s) {
            string subject = c.first;
            string parent = c.second;

            if (!onto->getConcept(parent)) {
                tmp[subject] = parent;
                continue;
            }

            cout << "Add concept " << subject << " as subconcept of " << parent << endl;
            onto->addConcept( subject, parent );
        }
        s = tmp;
    }



    for (auto& i : individuals) {
        for (auto obj : i.second) {
            if (obj.first != "NamedIndividual" && obj.second == "type") {
                cout << "Add instance " << i.first << " of type " << obj.first << endl;
                onto->addInstance(i.first, obj.first);
            }
        }
    }
}

void VROntology::open(string path) {
    raptor_world* world = raptor_new_world();
    raptor_parser* rdf_parser = raptor_new_parser(world, "rdfxml");
    unsigned char* uri_string = raptor_uri_filename_to_uri_string(path.c_str());
    raptor_uri* uri = raptor_new_uri(world, uri_string);
    raptor_uri* base_uri = raptor_uri_copy(uri);

    RDFdata RDFSubjects;
    raptor_parser_set_statement_handler(rdf_parser, &RDFSubjects, print_triple);
    raptor_parser_parse_file(rdf_parser, uri, base_uri);
    raptor_free_parser(rdf_parser);

    raptor_free_uri(base_uri);
    raptor_free_uri(uri);
    raptor_free_memory(uri_string);
    raptor_free_world(world);

    postProcessRDFSubjects(this, RDFSubjects);
}
