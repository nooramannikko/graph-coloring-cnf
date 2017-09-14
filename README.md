# Graph coloring application

This small program is written for TUT Artificial Intelligence course in 2016.

The program is very simple and therefore not very elegant. 
Main focus has not been on error management or user experience.

## Overview of the program

The program can be used to produce CNF formulas from given grpah inputs. 
The program takes a single text file and a coloring number as input and produces a new text file as output.
The input file is read one line at a time, and simultaneously the program builds a two-dimensional Boolean table to represent the graph structure. 
Each row or column in the graph represents a vertex in the graph, and the value true means that the intersecting vertices are connected. 
This graph is later used to build the actual CNF formula.

The formula is implemented with two kinds of Struct variables, Literals and Clauses. 
Literal represents a vertex that either is or is not colored with a specific color. 
For example X_12 means that the vertex labelled as 1 is colored with color number 2. 
Clauses are list of Literals, linked together with OR-operators. The formula is a list of Clauses, linked together with AND-operators. 
The formula is built in three phases plus a parsing phase. 
The phases are adapted from an [article](http://ceur-ws.org/Vol-533/09_LANMR09_06.pdf) by Ayanegui and Chavez-Aragon.

The clauses produced by all three phases are put together to form the formula. 
The used method produces a very long formula even for small graphs, and therefore it is probably not the most optimal way to solve the problem. 
However, the article used as reference seems reliable, and the phases used are quite simple and logical deep down.

Since the formula cannot be printed in the proper format with Struct variables, the formula is parsed before writing to a file. 
Each literal is replaced by an integer value, negative for negated literals. 
The value can be counted with the following formula:
```
vertex label + (color number - 1) * number of vertices
```

The produced output files were then given as input to a SAT solver called MiniSat.
