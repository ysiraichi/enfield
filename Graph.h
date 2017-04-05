#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <set>

class Graph {
    private:
        int n;
        std::vector< std::set<int> > successors;
        std::vector< std::set<int> > predecessors;
        std::set< std::pair<int, int> > reverseEdges;

    public:
        Graph(int n) : n(n) {
            successors.assign(n, std::set<int>());
            predecessors.assign(n, std::set<int>());
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

        std::set<int>& succ(int i) {
            return successors[i];
        }

        std::set<int>& pred(int i) {
            return predecessors[i];
        }

        bool hasEdge(int i, int j) {
            std::set<int>& succ = this->succ(i);
            return succ.find(j) != succ.end();
        }

        void putEdge(int i, int j) {
            std::set<int>& succ = this->succ(i);
            std::set<int>& pred = this->pred(j);

            succ.insert(j);
            pred.insert(i);
        }

        void buildReverseGraph() {
            for (int i = 0; i < n; ++i) {
                std::set<int>& succ = this->succ(i);

                for (int k : succ) {
                    if (!hasEdge(k, i)) {
                        putEdge(k, i);
                        reverseEdges.insert(std::pair<int, int>(k, i));
                    }
                }
            }
        }

        void print() {
            for (int i = 0; i < n; ++i) {
                const std::set<int> &succ = successors[i];
                const std::set<int> &pred = predecessors[i];

                std::cout << i << " -> ";
                for (int k : succ)
                    std::cout << "(" << k << ") ";
                std::cout << std::endl;

                std::cout << i << " <- ";
                for (int k : pred)
                    std::cout << "(" << k << ") ";
                std::cout << std::endl;
            }
        }
};

#endif
