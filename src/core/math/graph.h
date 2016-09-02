#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <vector>
#include <OpenSG/OSGVector.h>
#include "core/math/VRMathFwd.h"

using namespace std;
OSG_BEGIN_NAMESPACE;

class graph_base {
    public:
        enum CONNECTION {
            SIMPLE,
            HIERARCHY,
            SIBLING
        };

        struct node {
            Vec3f pos;
        };

        struct edge {
            int from;
            int to;
            CONNECTION connection;

            edge(int i, int j, CONNECTION c);
        };

    protected:
        vector< vector<edge> > edges;
        vector< node > nodes;

        virtual void update(int i);

    public:
        graph_base();
        ~graph_base();

        void connect(int i, int j, CONNECTION c = SIMPLE);
        node& getNode(int i);
        vector< node >& getNodes();
        vector< vector<edge> >& getEdges();

        void setPosition(int i, Vec3f v);
};

template<class T>
class graph : public graph_base {
    private:
        vector<T> elements;

        void update(int i);

    public:
        graph();
        ~graph();

        static shared_ptr< graph<T> > create();

        int addNode();
        int addNode(T t);

        vector<T>& getElements();
        T& getElement(int i);
};

OSG_END_NAMESPACE;

#endif // GRAPH_H_INCLUDED
