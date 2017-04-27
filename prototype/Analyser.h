#ifndef ANALYSER_H
#define ANALYSER_H

#include <string>
#include <vector>

std::vector< std::pair<int, int> > readDependencies(std::string filename, int &qubits);

enum Method { ISO, DYN, NONE };

extern std::string PhysFilename;
extern std::string ProgFilename;

#endif
