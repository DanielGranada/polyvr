#include "VRPathFinding.h"
#include "core/math/graph.h"
#include "core/math/path.h"
#include "core/utils/toString.h"

#include <OpenSG/OSGVector.h>
#include <map>

using namespace OSG;

VRPathFinding::Position::Position(int nID) : nID(nID) {}
VRPathFinding::Position::Position(int eID, float t) : eID(eID), t(t) {}

bool VRPathFinding::Position::operator==(const Position& p) {
    bool b1 = (nID == p.nID && eID == p.eID);
    bool b2 = (abs(t-p.t) < 1e-6);
    return b1 && b2;
}

bool VRPathFinding::Position::operator<(const Position& p) const {
    float k1 = nID + eID + t;
    float k2 = p.nID + p.eID + p.t;
    return k1 < k2;
}

string VRPathFinding::Position::toString() { return nID < 0 ? "edge "+::toString(eID)+" at "+::toString(t) : "node "+::toString(nID); }

Vec3d VRPathFinding::pos(Position& p) {
    if (p.nID >= 0) {
        auto node = graph->getNode(p.nID);
        return node.p.pos();
    }

    if (p.eID >= 0) {
        auto path = paths[p.eID];
        return path->getPose(p.t)->pos();
    }

    return Vec3d();
}


VRPathFinding::VRPathFinding() {}
VRPathFinding::~VRPathFinding() {}

VRPathFindingPtr VRPathFinding::create() { return VRPathFindingPtr( new VRPathFinding() ); }

void VRPathFinding::setGraph(GraphPtr g) { graph = g; }
void VRPathFinding::setPaths(vector<pathPtr> p) { paths = p; }

float VRPathFinding::getDistance(Position n1, Position n2) {
    return (pos(n2) - pos(n1)).length(); // ? C++
}


vector<VRPathFinding::Position> VRPathFinding::reconstructBestPath(Position current){
    vector<Position> bestRoute( { current } );
    while( cameFrom.count(current) ) {
        current = cameFrom[current];
        bestRoute.push_back(current);
    }
    reverse(bestRoute.begin(), bestRoute.end());
    return bestRoute;
}

bool VRPathFinding::valid(Position& p) {
    if (p.nID >= 0) if (graph->hasNode(p.nID)) return true;
    if (p.eID >= 0 && p.t >= 0 && p.t <= 1) if (graph->hasEdge(p.eID)) return true;
    return false;
}

VRPathFinding::Position VRPathFinding::getMinFromOpenSet() {
    Position res;
    float fMin = 1e9;
    for (auto n : openSet) {
        float f = fCost[n.first];
        if (f < fMin) {
            fMin = f;
            res = Position(n.first);
        }
    }
    return res;
}

vector<VRPathFinding::Position> VRPathFinding::getNeighbors(Position& p) {
    vector<Position> res;
    if (!graph) return res;
    if (graph->hasNode(p.nID) && int(graph->getEdges().size()) > p.nID) {
        for (auto edge : graph->getEdges()[p.nID]) {
            if (edge.from != p.nID) res.push_back(Position(edge.from));
            if (edge.to != p.nID) res.push_back(Position(edge.to));
        }
    }
    if (graph->hasEdge(p.eID)) {
        auto edge = graph->getEdge(p.eID);
        res.push_back(Position(edge.from));
        res.push_back(Position(edge.to));
    }
    return res;
}

vector<VRPathFinding::Position> VRPathFinding::computePath(Position start, Position goal) {
    if (!valid(start) || !valid(goal)) {
        string p = valid(start)?"goal":"start";
        cout << "VRPathFinding::computePath Error: " << p << " position invalid!" << endl;
        return vector<Position>();
    }

    map<Position, float> gCost; //key = node, value = cost from the start to current node

    closedSet.clear();
    openSet.clear();
    cameFrom.clear();
    fCost.clear();

    openSet[start] = 1;
    while (!openSet.empty()) {
        Position current = getMinFromOpenSet();
        if (current == goal) return reconstructBestPath(current);

        openSet.erase(current);
        closedSet[current] = 1;

        for (auto neighbor : getNeighbors(current)) { // TODO: take edge positions into account!
            if (closedSet.count(neighbor)) continue;
            gCost[current] = getDistance(start, current);
            float tentative_gCost = gCost[current] + getDistance(current, neighbor);

            if (!openSet.count(neighbor)) openSet[neighbor] = 1;
            else if (tentative_gCost >= gCost[neighbor]) continue;

            cameFrom[neighbor] = current;
            gCost[neighbor] = tentative_gCost;
            fCost[neighbor] = gCost[neighbor] + getDistance(neighbor, goal); //heuristic_cost_estimate(node, end)
        }
    }
    cout << "VRPathFinding::computePath Error: no route found from " << start.toString() << " to " << goal.toString() << endl;
    return vector<Position>();
}

