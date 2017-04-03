#include "Graph.h"
#include "Isomorphism.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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

void mapForEach(vector<int> mapping, string filename) {
    ifstream ifs(filename.c_str());

    int n;
    ifs >> n;

    vector< pair<int, int> > dependencies;
    for (int u, v; ifs >> u >> v;) {
        dependencies.push_back(pair<int, int>(u, v));
    }

    for (int t = 0, e = dependencies.size(); t < e; ++t) {

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
    for (int i = 0; i < mapping.size(); ++i) {
        if (mapping[i] != -1)
            cout << i << " -> " << mapping[i] << endl;
    }

    return 0;
}
