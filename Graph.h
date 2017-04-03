#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <set>

class Graph {
    private:
        int n;
        std::vector< std::vector<int> > successors;
        std::vector< std::vector<int> > predecessors;
        std::set< std::pair<int, int> > reverseEdges;

    public:
        Graph(int n) : n(n) {
            successors.assign(n, std::vector<int>());
            predecessors.assign(n, std::vector<int>());
        }

        int inDegree(int i) const {
            return predecessors[i].size();
        }

        int outDegree(int i) const {
            return successors[i].size();
        }

        int size() const {
            return n;
        }

        std::vector<int>& succ(int i) {
            return successors[i];
        }

        std::vector<int>& pred(int i) {
            return predecessors[i];
        }

        bool hasEdge(int i, int j) {
            std::vector<int>& succ = this->succ(i);
            std::vector<int>::iterator it = std::find(succ.begin(), succ.end(), j);
            return it != succ.end();
        }

        void putEdge(int i, int j) {
            std::vector<int>& succ = this->succ(i);
            std::vector<int>& pred = this->pred(j);

            succ.push_back(j);
            pred.push_back(i);
        }

        void buildReverseGraph() {
            for (int i = 0; i < n; ++i) {
                std::vector<int>& succ = this->succ(i);

                for (int j = 0, f = succ.size(); j < f; ++j) {
                    int k = succ[j];
                    if (!hasEdge(k, i)) {
                        putEdge(k, i);
                        reverseEdges.insert(std::pair<int, int>(k, i));
                    }
                }
            }
        }

        void print() {
            for (int i = 0; i < n; ++i) {
                const std::vector<int> &succ = successors[i];
                const std::vector<int> &pred = predecessors[i];

                std::cout << i << " -> ";
                for (int i = 0, e = succ.size(); i < e; ++i) {
                    std::cout << succ[i];
                    if (i < e-1) std::cout << ", ";
                }
                std::cout << std::endl;

                std::cout << i << " <- ";
                for (int i = 0, e = pred.size(); i < e; ++i) {
                    std::cout << pred[i];
                    if (i < e-1) std::cout << ", ";
                }
                std::cout << std::endl;
            }
        }
};

#endif
