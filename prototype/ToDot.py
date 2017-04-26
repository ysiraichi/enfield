#! /usr/bin/python3

import sys

filename = sys.argv[1]
output = sys.argv[2]

fin = open(filename)
fout = open(output, 'w')

fout.write("digraph " + filename + " {\n")

n = int(fin.readline());
lines = fin.readlines()

for line in lines:
    edge = line.replace('\n', '').split(' ')
    lineOut = "\t" + str(edge[0]) + " -> " + str(edge[1]) + ";\n"
    fout.write(lineOut)

fout.write("}\n")

fin.close()
fout.close()
