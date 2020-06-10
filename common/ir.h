/***
 * 
 *  IR As a Whole Definition
 *  The class OIR simply represents a whole program for compilation
 * 
 *  Design Goal:
 *  Simple to implement
 *  Optimisations Friendly
 *  Similar to WHIRL
 *  Multi-level
 *  Easily convertible to WHIRL / Maple / LLVM-IR
 *  Generating code that run faster than competitors (but with easy optimizations only)
 *  Correctly running code under SYSY semantics, on ARMv7 machine
 *  
 *  Tradeoffs:
 *  Using more inner-memory data structure : will cause more difficult binary file dumping. -> Solvable through API isolation
 *  Using STL templates more, reduncing development time, harder MMAP or data swaping for disk
 *  Less types & opcodes supported : Eaiser implement but less functional
 * 
 *  Non-Goals:
 *  Memory efficient, not freeing anything if not producing too much memory footprint
 *  Fully C99/C11/C++0x/C++11/C++17 compatible
 *  Generating optimal code
 *  Using complex machine instructions (such as Neon)
 *  
 *  
 *  
 * */
#include "symtab.h"
#include "tree.h"
