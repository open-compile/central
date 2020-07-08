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
#include "ir.h"

/**
 * Printing this node
 * @param f
 */
void IRNODE::Print(FILE *f) {
  fprintf(f, "%s", OPCODE_name(opcode));
  switch (opcode) {
    case OPC_I4STID: {
      if (this->Get_symbol_idx() != 0) {
        fprintf(f, "  var<%s, idx = %0#x, level = %d, tabid = %d>",
          ST_name(this->Get_symbol_idx()),
          this->Get_symbol_idx(),
          this->Get_symbol_idx() & 0xff,
          this->Get_symbol_idx() >> 8);
      } else {
        fprintf(f, "  var<%u>", this->Get_symbol_idx());
      }
      break;
    }
    case OPC_I4CONST: {
      fprintf(f, " const<hex = %0#x, int = %d>",
              (INT32) this->Get_const_val(),
              (INT32) this->Get_const_val());
      break;
    }
    default: {
      // ... nothing to do
    }
  }
}

const char *IRNODE::OPCODE_name(OPCODE opcode) {
  switch (opcode) {
    case OPC_I4I4ADD:
      return "I4I4ADD";
    case OPC_I4I4SUB:
      return "I4I4SUB";
    case OPC_I4I4MPY:
      return "I4I4MPY";
    case OPC_I4I4MOD:
      return "I4I4MOD";
    case OPC_FUNC_ENTRY:
      return "FUNC_ENTRY";
    case OPC_BLOCK:
      return "BLOCK";
    case OPC_I4STID:
      return "I4STID";
    case OPC_I4LDID:
      return "I4LDID";
    case OPC_I4I4ISTORE:
      return "I4I4ISTORE";
    case OPC_I4I4ILOAD:
      return "I4I4ILOAD";
    case OPC_IF:
      return "IF";
    case OPC_I4CONST:
      return "I4CONST";
    case OPC_I8CONST:
      return "I8CONST";
    default: {
      return "UNKNOWN-OPCODE";
    }
  }
}

IRNODE_IDX &IRNODE::Opnd(UINT32 pos) {
  AssertThat(pos == 0 || pos == 1, ("Operand must be zero or one"));
  return this->u3.kids[pos];
}

static TREE *current_tree = NULL;

/**
 * Global value for accessing the current function
 * @return
 */
TREE *Tree() {
  AssertThat(current_tree != NULL, ("Current tree is NULL"));
  return current_tree;
}

void Set_current_tree(TREE *current) {
  AssertThat(current != NULL, ("The tree to be used is NULL"));
  current_tree = current;
}

void TREE::Print_recursive(FILE *f) {
  fprintf(f, "Printing the table size = %lu, tree of size = %zu ... \n", _ir_elem_tab.size(), irtree.size());
  IR_ITER it = irtree.begin();
  for (; it != irtree.end(); it++) {
    fprintf(f, "IDX[%llu] ", *it);
    for(UINT32 i = 0; i < irtree.depth(it); i++) {
      fprintf(f, "-");
    }
    Get_node(*it)->Print(f);
    fprintf(f, "      - kid(%d), index(%u), depth(%u)\n",
    it.number_of_children(), irtree.index(it), irtree.depth(it));
  }
}

IR_ITER TREE::Get_root() {
  return irtree.begin();
}

IRNODE *TREE::Get_node(IRNODE_IDX iridx) {
  return &_ir_elem_tab[iridx];
}

IRNODE_IDX TREE::Create_node() {
  IRNODE irnode(OPC_I4I4ADD);
  IRNODE_IDX ir_elem_idx = _ir_elem_tab.size();
  _ir_elem_tab.push_back(irnode);
  return ir_elem_idx;
}

IR_ITER TREE::Insert_stmt_to_block(IR_ITER block, IR_ITER child) {
  return irtree.append_child(block, child);
}

IR_ITER TREE::Insert_stmt_to_block(IR_ITER block, IR_TREE_ELEM child) {
  return irtree.append_child(block, child);
}

IR_ITER TREE::Set_operand(IR_ITER parent, UINT32 pos, IR_ITER opnd) {
  AssertThat(irtree.number_of_children(parent) >= pos, ("Not enough children in parent node"));
  AssertThat(parent != opnd, ("Cannot set the node to be the child of itself"));
  Get_node(*parent)->Opnd(pos) = *opnd;
  while (irtree.number_of_children(parent) < pos + 1) {
    IR_ITER fake_addeed_opr = irtree.append_child(parent, 0);
    Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
      (TFile, "Adding dummy child to parent = %llu\n", *parent));
  }
  IR_ITER fake_child = irtree.child(parent, pos);
  IR_ITER res = irtree.move_ontop(fake_child, opnd);
  return res;
}

IR_ITER TREE::Set_operand(IR_ITER parent, UINT32 pos, IR_TREE_ELEM opnd) {
  Get_node(*parent)->Opnd(pos) = opnd;
  while (irtree.number_of_children(parent) < pos + 1) {
    IR_ITER fake_addeed_opr = irtree.append_child(parent, 0);
  }
  IR_ITER fake_child = irtree.child(parent, pos);
  irtree.erase_children(fake_child); //remove original children
  return irtree.replace(fake_child, opnd);
}

IR_ITER TREE::Get_operand(IR_ITER node_iter, UINT32 kid_pos) {
  AssertThat(node_iter.number_of_children() > kid_pos, ("Insufficient number of kid for node = %d", *node_iter))
  return irtree.child(node_iter, kid_pos);
}

// Do nothing
void TREE::Initialize() {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO), (TFile, "Creating a TREE.Initialize() ... \n"));
  IRNODE_IDX func_entry = Create_node(OPC_FUNC_ENTRY);
  IRNODE_IDX pragmas = Create_node(OPC_BLOCK);
  IRNODE_IDX body = Create_node(OPC_BLOCK);
  IR_ITER r = Set_root(func_entry);
  Set_operand(r, TREE_SEQ_BODY, body);
  Set_operand(r, TREE_SEQ_PRAGMA, pragmas);
  AssertThat(*(Get_root()) == func_entry, ("Failed setting root to tree"));
  AssertThat((* Get_operand(r, TREE_SEQ_BODY)) == body, ("Failed setting operand to parent"));
  AssertThat((* Get_operand(r, TREE_SEQ_PRAGMA)) == pragmas, ("Failed setting operand to parent"));
}

IR_ITER TREE::Set_root(IR_TREE_ELEM root_node) {
  return irtree.set_head(root_node);
}

IRNODE_IDX TREE::Create_node(OPCODE opc) {
  IRNODE_IDX node = Create_node();
  Get_node(node)->opcode = opc;
  return node;
}

IR_ITER TREE::Add_child(IR_ITER parent, IR_TREE_ELEM child) {
  return irtree.append_child(parent, child);
}

IR_ITER TREE::Insert_temp_node(IRNODE_IDX idx) {
  return irtree.insert(irtree.begin_breadth_first(), idx);
};

///**
// * Utility function for getting the item from the Tree()
// * @param elem
// * @return
// */
//IRNODE &TNode(IR_TREE_ELEM elem) {
//  return *(Tree()->Get_node(elem));
//}
