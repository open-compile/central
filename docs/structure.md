# Compiler Structure in OCC

### Overview
The OCC main architecture contains four parts:
1. driver
2. front end
3. back end
4. code generator
5. linker invocator(using ld capabilities)

### Front-end design 
Input: Source code, Output: High-level OCCIR, with loop/high level control structures intact
1. Firstly, run bison + flex doing the lexical analysis to construct AST.
2. Conduct once a DFS traversal on all function ASTs and var declarations to generate OCC-IR...
3. Perform correctness check on the result OCC-IR tree.

### Back-end design
Input: High level language OCCIR, Output: Very-low level CGIR containing three-item-tuple like stmts.
Procedure: Consecutive lowering of high-mid-low levels of OCCIR 
Optimizations/Transformations done in each level:
- Very high -> High: 
- High -> Mid: (FALSEBR, TRUEBR should be not be here...)
- Mid -> Low: 
  - Lowering IF/While-do structure to FALSEBR,TRUEBR
  - Comma structure flattening
  - LDID-const opt
  - 
- Low -> Very low:
  - LAND, BIOR lowering to FALSEBR
  - 
- Very low -> CGIR:  

Optimization/Transformation done on every level:
- Constant propagation / folding ....
- 

### Code Generator design
The code generator does the following routines:
1. Layout preparation
2. Local/Global register allocation
   - callculate register inference graph
   - coloring
   - spilling & iterating...
3. Layout refinement
4. 

### OCC-IR design
The OCC IR is made up of several components:
- 

### Supported language(s)
Currently, OCC supports compiling a subset of C, referred to as sysy language.
A specific definition of sysy language can be found online, outside of this repository.

### TODO list
1. Compiling to LLVM IR / MLIR...
2. Cross-check the results with LLVM code generation, comparison on basic testcases.
