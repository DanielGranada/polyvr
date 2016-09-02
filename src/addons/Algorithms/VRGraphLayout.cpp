#include "VRGraphLayout.h"
#include "core/math/Octree.h"
#include "core/math/graphT.h"

using namespace OSG;

template class graph<VRGraphLayout::Node>;

void VRGraphLayout::Node::update(graph_base::node& n) {
    Vec3f d = n.pos - bb.center();
    bb.move( d );
}

VRGraphLayout::VRGraphLayout() {}

void VRGraphLayout::setGraph(layout& g) { this->g = g; }
VRGraphLayout::layout& VRGraphLayout::getGraph() { return g; }
void VRGraphLayout::setAlgorithm(ALGORITHM a, int position) { algorithms[position] = a; }
void VRGraphLayout::clearAlgorithms() { algorithms.clear(); }

void VRGraphLayout::applySprings(float eps) {
    for (auto& n : g.getEdges()) {
        for (auto& e : n) {
            auto f1 = getFlag(e.from);
            auto f2 = getFlag(e.to);
            Node& n1 = g.getElement(e.from);
            Node& n2 = g.getElement(e.to);
            Vec3f p1 = n1.bb.center();
            Vec3f p2 = n2.bb.center();

            float r = radius + n1.bb.radius() + n2.bb.radius();
            Vec3f d = p2 - p1;
            float x = (d.length() - r)*eps; // displacement
            d.normalize();
            if (abs(x) < eps) continue;

            if (x > r*eps) x = r*eps; // numerical safety ;)
            if (x < -r*eps) x = -r*eps;

            Vec3f grav = gravity*x*0.1; // TODO: not yet working!
            switch (e.connection) {
                case layout::SIMPLE:
                    if (f1 != FIXED) g.setPosition( e.from, p1 + d*x*r + grav );
                    if (f2 != FIXED) g.setPosition( e.to, p2 - d*x*r + grav );
                    break;
                case layout::HIERARCHY:
                    if (f2 != FIXED) g.setPosition( e.to, p2 - d*x*r + grav );
                    else if (f1 != FIXED) g.setPosition( e.from, p1 + d*x*r + grav );
                    break;
                case layout::SIBLING:
                    if (x < 0) { // push away siblings
                        if (f1 != FIXED) g.setPosition( e.from, p1 + d*x*r + grav );
                        if (f2 != FIXED) g.setPosition( e.to, p2 - d*x*r + grav );
                    }
                    break;
            }
        }
    }
}

void VRGraphLayout::applyOccupancy(float eps) {
    auto& nodes = g.getElements();
    Octree o(10*eps);

    long i=0;
    for (auto& n : nodes) {
        o.add( OcPoint(n.bb.center(), (void*)i) );
        i++;
    }

    for (int i=0; i<nodes.size(); i++) {
        auto& n = nodes[i];
        Vec3f pn = n.bb.center();
        float rs = radius + 2*n.bb.radius();
        if ( getFlag(i) == FIXED ) continue;

        Vec3f D;
        for (auto& on2 : o.radiusSearch(pn, rs) ) {
            int i = (long)on2;
            auto& n2 = nodes[i];
            Vec3f d = n2.bb.center() - pn;
            float r = radius + n.bb.radius() + n2.bb.radius();
            float x = (r - d.length())*eps; // displacement
            if (x > 0) {
                d.normalize();
                D -= d*x*r*0.5;
            }
        }

        g.setPosition(i, pn+D); // move node away from neighbors
    }
}

void VRGraphLayout::compute(int N, float eps) {
    for (int i=0; i<N; i++) { // steps
        for (auto a : algorithms) {
            switch(a.second) {
                case SPRINGS:
                    applySprings(eps);
                    break;
                case OCCUPANCYMAP:
                    applyOccupancy(eps);
                    break;
            }
        }
    }
}

VRGraphLayout::FLAG VRGraphLayout::getFlag(int i) { return flags.count(i) ? flags[i] : NONE; }
void VRGraphLayout::fixNode(int i) { flags[i] = FIXED; }

void VRGraphLayout::setRadius(float r) { radius = r; }
void VRGraphLayout::setGravity(Vec3f v) { gravity = v; }
