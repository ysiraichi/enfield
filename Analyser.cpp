#include "Graph.h"
#include "Isomorphism.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

Graph* readGraph(string filename) {
    ifstream ifs(filename.c_str());

    int n;
    Graph *graph;

    ifs >> n;

    graph = new Graph(n);

    for (int u, v; ifs >> u >> v;) {
        graph->succ(u).push_back(v);
        graph->pred(v).push_back(u);
    }

    ifs.close();
    return graph;
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

        vector<int> &succ = physGraph.succ(x);
        for (int i = 0, e = succ.size(); i < e; ++i) {
            if (!marked[succ[i]]) {
                q.push(succ[i]);
                marked[succ[i]] = true;
                parent[succ[i]] = x;
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

void swapPath(Graph &physGraph, vector<int> &mapping, vector<int> path) {
    vector<int> assigned(physGraph.size(), -1);

    for (int i = 0, e = mapping.size(); i < e; ++i)
        if (mapping[i] != -1)
            assigned[mapping[i]] = i;

    for (int i = 1, e = path.size(); i < e; ++i) {
        // (u, v) in Phys
        int u = path[i-1], v = path[i];
        // (a, b) in Prog
        int a = assigned[u], b = assigned[v];

        cout << "Swapping (" << a << ", " << b << ")" << endl;

        assigned[u] = b;
        assigned[v] = a;

        mapping[a] = v;
        mapping[b] = u;

        path[i-1] = v;
        path[i] = u;
    }
}

vector< vector<int> > mapForEach(Graph &physGraph, vector<int> mapping, string filename) {
    ifstream ifs(filename.c_str());

    int n;
    ifs >> n;

    vector< pair<int, int> > dependencies;
    for (int u, v; ifs >> u >> v;) {
        dependencies.push_back(pair<int, int>(u, v));
    }

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
            swapPath(physGraph, current, path);
            mappings.push_back(current);
        }

        cout << "Dep: " << dep.first << " -> " << dep.second << endl;

    }

    return mappings;
}

void printMapping(vector<int> &mapping) {
    cout << "Prog -> Phys" << endl;

    for (int i = 0; i < mapping.size(); ++i) {
        if (mapping[i] != -1)
            cout << i << " -> " << mapping[i] << endl;
    }
}

int main(int argc, char **argv) {
    string physFilename = argv[1];
    string progFilename = argv[2];

    Graph *physGraph = readGraph(physFilename);
    physGraph->buildReverseGraph();
    physGraph->print();

    cout << endl;

    Graph *progGraph = readGraph(progFilename);
    progGraph->print();

    cout << endl;

    vector<int> mapping = findIsomorphism(*physGraph, *progGraph);
    printMapping(mapping);

    cout << "--------------------------------" << endl;

    vector< vector<int> > mappings = mapForEach(*physGraph, mapping, progFilename);

    cout << "--------------------------------" << endl;

    for (int i = 0, e = mappings.size(); i < e; ++i)
        printMapping(mappings[i]);

    return 0;
}
