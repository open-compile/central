/**
 *  Copyright SZU Compiler Team 2020 
 *  Author : @Guanting.Lu
 * 
 *  This is where most of the important definitions are presented
 *  we define the tree for containing the semantic content of the program we are about to compile
 *  
 *  Data Stuctures
 *  OIR
 *   TREE
 *   TY_TABLE
 *   ST_TABLE
 *   PREG_TABLE
 *   LABEL_TABLE
 *   ... 
 *  
 *  
 * 
 * */
#include "tree.h"

TREE *TREE::Create() {
  return new TREE();
}
