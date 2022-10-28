# DynamicSymbolicExecution
Assignment for implementing dynamic symbolic execution.

Dynamic Symbolic execution is a form of automated software testing, where we systematically generate inputs that 
can explore all paths in the program.

The tasks completed in this assignment include:
1. Implemented **src/Instrument.cpp** to automatically add DSE-enabling functions during compilation.
2. Created the implementations of the DSE-enabling functions in **src/Runtime.cpp**
3. Implemented a backtracking strategy in **src/Strategy.cpp** where, after each execution of the program, we analyze the 
paths covered by the current input and propose a new input that can explore a different path. This contains the main 
logic of the DSE.
4. **src/DSE.cpp**, **src/SymbolicInterpreter.cpp** and **header files** were provided and they contained some starter code 
and utility functions.

The full assignment description is in Assignment.pdf.

To test the assignment, clone this repository and follow the steps in the "Setup" section of Assignment.pdf
