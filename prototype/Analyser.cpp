#include "Analyser.h"
#include "Graph.h"
#include "Isomorphism.h"
#include "DynSolver.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

using namespace std;

enum Method M;

std::string PhysFilename;
std::string ProgFilename;

std::vector<std::string> PhysVarNames;
std::vector<std::string> ProgVarNames;

const int SwapCost = 7;
const int RevCost = 4;

Graph* readGraph(string filename, vector<string> &var) {
    ifstream ifs(filename.c_str());

    int n;
    Graph *graph;

    ifs >> n;

    var.assign(n, "");
    graph = new Graph(n);

    int label = 0;
    std::unordered_map<string, int> idxMap;
    for (string u, v; ifs >> u >> v;) {
        if (idxMap.find(u) == idxMap.end()) idxMap[u] = label++;
        if (idxMap.find(v) == idxMap.end()) idxMap[v] = label++;
        graph->putEdge(idxMap[u], idxMap[v]);
    }

    for (auto pair : idxMap)
        var[pair.second] = pair.first;

    ifs.close();
    return graph;
}

vector< pair<int, int> > readDependencies(string filename, int &qubits) {
    ifstream ifs(filename.c_str());

    ifs >> qubits;

    int label = 0;
    unordered_map<string, int> idxMap;
    vector< pair<int, int> > dependencies;
    for (string u, v; ifs >> u >> v;) {
        if (idxMap.find(u) == idxMap.end()) idxMap[u] = label++;
        if (idxMap.find(v) == idxMap.end()) idxMap[v] = label++;
        dependencies.push_back(pair<int, int>(idxMap[u], idxMap[v]));
    }

    ifs.close();
    return dependencies;
}

int getFreeVertex(Graph &physGraph, vector<int> mapping) {
    int n = physGraph.size();

    bool assigned[n];
    for (int i = 0; i < n; ++i)
        assigned[i] = false;

    for (int i = 0, e = mapping.size(); i < e; ++i)
        if (mapping[i] != -1)
            assigned[mapping[i]] = true;

    for (int i = 0; i < n; ++i)
        if (!assigned[i])
            return i;

    return -1;
}

vector<int> getPath(Graph &physGraph, int u, int v) {
    vector<int> parent(physGraph.size(), -1);
    vector<bool> marked(physGraph.size(), false);

    queue<int> q;
    q.push(u);
    marked[u] = true;

    while (!q.empty()) {
        int x = q.front();
        q.pop();

        if (x == v) break;

        set<int> &succ = physGraph.succ(x);
        for (int k : succ) {
            if (!marked[k]) {
                q.push(k);
                marked[k] = true;
                parent[k] = x;
            }
        }
    }

    int x = v;
    vector<int> path;
    do {
        path.push_back(x);
        x = parent[x];
    } while (parent[x] != -1);

    return path;
}

void swapPath(Graph &physGraph, vector<int> &mapping, vector<int> path, ostream &out) {
    vector<int> assigned(physGraph.size(), -1);

    for (int i = 0, e = mapping.size(); i < e; ++i)
        if (mapping[i] != -1)
            assigned[mapping[i]] = i;

    for (int i = 1, e = path.size(); i < e; ++i) {
        // (u, v) in Phys
        int u = path[i-1], v = path[i];
        // (a, b) in Prog
        int a = assigned[u], b = assigned[v];

        out << "Swapping (" << a << ", " << b << ")" << endl;

        assigned[u] = b;
        assigned[v] = a;

        if (a != -1)
            mapping[a] = v;
        if (b != -1)
            mapping[b] = u;

        path[i-1] = v;
        path[i] = u;
    }
}

vector< vector<int> > mapForEach(Graph &physGraph, vector<int> mapping, int &cost) {
    int qubits;
    vector< pair<int, int> > dependencies = readDependencies(ProgFilename, qubits);

    cost = 0;

    vector< vector<int> > mappings = { mapping };
    for (int t = 0, e = dependencies.size(); t < e; ++t) {
        pair<int, int> dep = dependencies[t];
        vector<int> current = mappings.back();

        int u = current[dep.first], v = current[dep.second];
        if (u == -1) {
            u = getFreeVertex(physGraph, current);
            current[dep.first] = u;
        }

        if (v == -1) {
            v = getFreeVertex(physGraph, current);
            current[dep.second] = v;
        }

        vector<int> path = getPath(physGraph, u, v);
        if (path.size() > 1) {
            cost += ((path.size() - 1) * SwapCost);

            swapPath(physGraph, current, path, cerr);
            mappings.push_back(current);
        }

        if (physGraph.isReverseEdge(u, v))
            cost += RevCost;

        cerr << "Dep: " << dep.first << " -> " << dep.second << endl;

    }

    return mappings;
}

void printMapping(vector<int> &mapping) {
    cout << "Prog -> Phys" << endl;

    for (int i = 0; i < mapping.size(); ++i) {
        if (mapping[i] != -1)
            cout << ProgVarNames[i] << " -> " << PhysVarNames[mapping[i]] << endl;
    }
}

void readArgs(int argc, char **argv) {
    PhysFilename = argv[1];
    ProgFilename = argv[2];

    M = NONE;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-iso")) M = ISO;
        if (!strcmp(argv[i], "-dyn")) M = DYN;
    }

    if (M == NONE) M = DYN;
}

std::string toStrPhysGraph(int i) {
    return PhysVarNames[i];
}

std::string toStrProgGraph(int i) {
    return ProgVarNames[i];
}

int main(int argc, char **argv) {
    readArgs(argc, argv);

    Graph *physGraph = readGraph(PhysFilename, PhysVarNames);
    physGraph->buildReverseGraph();
    physGraph->setToStrFn(toStrPhysGraph);
    physGraph->print();

    cout << endl;

    Graph *progGraph = readGraph(ProgFilename, ProgVarNames);
    progGraph->setToStrFn(toStrProgGraph);
    progGraph->print();

    cout << endl;
    vector< vector<int> >  mappings;

    cout << "--------------------------------" << endl;
    vector<int> mapping;

    switch (M) {
        case ISO:
            {
                mapping = findIsomorphism(*physGraph, *progGraph);
                break;
            }

        case DYN:
            {
                mapping = dynsolve(*physGraph);
            }

        default:
            break;
    }

    int totalCost;

    printMapping(mapping);
    mappings = mapForEach(*physGraph, mapping, totalCost);

    cout << "--------------------------------" << endl;
    for (int i = 0, e = mappings.size(); i < e; ++i) {
        printMapping(mappings[i]);
        cout << endl;
    }

    cout << "Cost: " << totalCost << endl;

    return 0;
}
