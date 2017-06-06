#ifndef DYN_SOLVER_H
#define DYN_SOLVER_H

#include "Graph.h"
#include "Analyser.h"

MapResult dynsolve(Graph &physGraph); 

extern const int SwapCost;
extern const int RevCost;

#endif
