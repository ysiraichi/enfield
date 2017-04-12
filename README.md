# Enfield
-----------------------------------------------------

For compiling, use the ```make``` command.

It will generate an executable called ```enfield```. This executable needs two parameters for correct execution:

* The physical architecture connectivity graph;
* The programs dependency graph in chronological order. 

Enfield has two modes of execution: ```dyn``` and ```iso```. Both are used to find the best initial mapping.


| Method | Description |
| ------ | ------ |
| ```-iso``` | Tries to find an isomorphism of the whole program, where the best initial configuration is set to the subgraph isomorphism with the least errors. |
| ```-dyn``` | This is a dynamic programming approach that tests all possibilities. |

Note that both uses a BFS algorithm in order to find a path between two vertices.

