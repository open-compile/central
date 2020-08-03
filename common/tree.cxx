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
  switch (OPCODE_operator(opcode)) {
    case OPR_STID:
    case OPR_LDID: {
      if (this->Get_symbol_idx() != 0) {
        fprintf(f, "  var<%s, idx = %0#x, level = %d, tabid = %d, ofst = %d>",
          ST_name(this->Get_symbol_idx()),
          this->Get_symbol_idx(),
          this->Get_symbol_idx() & 0xff,
          this->Get_symbol_idx() >> 8,
          this->Get_load_offset());
      } else {
        fprintf(f, "  var<%u>", this->Get_symbol_idx());
      }
      break;
    }
    case OPR_CONST: {
      fprintf(f, " const<hex = %0#x, int = %d>",
              (INT32) this->Get_const_val(),
              (INT32) this->Get_const_val());
      break;
    }
    case OPR_TRUEBR:
    case OPR_FALSEBR:
    case OPR_LABEL:
    case OPR_GOTO: {
      fprintf(f, " <label id = 0x%06x, or %u> ", (UINT32) this->Get_label_num(), (UINT32) this->Get_label_num());
      break;
    }
    case OPR_GOTO_OUT: {
      fprintf(f, " <%s> ", Get_label_num() == GOTO_OUT_BREAK ? "break" : "continue");
      break;
    }
    case OPR_CALL: {
      fprintf(f, " <callee = %d, name = %s> ", Get_symbol_idx(), ST_name(Get_symbol_idx()));
      break;
    }

    case OPR_ARRAY: {
      fprintf(f, " dimensions = %d ",
              (INT32) this->Get_const_val());
      break;
    }
    case OPR_LDA: {
      if (this->Get_symbol_idx() != 0) {
        fprintf(f, "  var<%s, idx = %0#x, level = %d, tabid = %d, ofst = %d>",
                ST_name(this->Get_symbol_idx()),
                this->Get_symbol_idx(),
                this->Get_symbol_idx() & 0xff,
                this->Get_symbol_idx() >> 8,
                this->Get_load_offset());
      } else {
        fprintf(f, "  var<%u>", this->Get_symbol_idx());
      }
      break;
    }
    default: {
      // ... nothing to do
    }
  }
}

OPCODE_INFO opc_info_table[] = {
#define OCIR_OPC(opc_enum, opr, mtype_res, mtype_desc) \
  {opc_enum, opr, mtype_res, mtype_desc, #opc_enum},
#include "opc_base.h"
#undef OCIR_OPC
};

const char *OPCODE_name(OPCODE opcode) {
  // TODO: Print it by someway else
  // Print mtype DESC
  // Print mtype RES
  // Print OPR
  UINT32      count = sizeof(opc_info_table) / sizeof(OPCODE_INFO);
  for (UINT32 i     = 0; i < count; i++) {
    if (opc_info_table[i].opc == opcode)
      return opc_info_table[i].name;
  }
  return "UNKOWN";
}

IRNODE_IDX &IRNODE::Opnd(UINT32 pos) {
  AssertThat(pos == 0 || pos == 1, ("Operand must be zero or one"));
  return this->extra3.kids[pos];
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

void TREE::Print() {
  this->Print_recursive(stdout);
}

void TREE::Print_recursive(FILE *f) {
  fprintf(f, "Printing the table size = %lu, tree of size = %zu ... \n", _ir_elem_tab.size(), irtree.size());
  IR_ITER it = irtree.begin();
  for (; it != irtree.end(); it++) {
    fprintf(f, "IDX[%-8llu] ", *it);
    for(UINT32 i = 0; i < irtree.depth(it); i++) {
      fprintf(f, "-");
    }
    Get_node(*it)->Print(f);
    fprintf(f, "      - kid(%d), index(%u), depth(%u)\n",
    it.number_of_children(), irtree.index(it), irtree.depth(it));
  }
}

IR_ITER TREE::Get_root() {
  AssertThat(this != nullptr, ("The tree should not be null."));
  return irtree.begin();
}

IRNODE *TREE::Get_node(IRNODE_IDX iridx) {
  AssertThat(this != nullptr, ("The tree should not be null."));
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
  if (pos == 0 || pos == 1)
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
  if (pos == 0 || pos == 1)
    Get_node(*parent)->Opnd(pos) = opnd;
  while (irtree.number_of_children(parent) < pos + 1) {
    IR_ITER fake_addeed_opr = irtree.append_child(parent, 0);
  }
  IR_ITER fake_child = irtree.child(parent, pos);
  irtree.erase_children(fake_child); //remove original children
  return irtree.replace(fake_child, opnd);
}

IR_ITER TREE::Get_operand(IR_ITER node_iter, UINT32 kid_pos) {
  if (node_iter.number_of_children() <= kid_pos) {
    fprintf(stderr, "Problematic node: ");
    Get_node(*node_iter)->Print(stderr);
    fprintf(stderr, "\n");
    this->Print_recursive(stderr);
  }
  AssertThat(node_iter.number_of_children() > kid_pos,
             ("Insufficient number of kid for node = %d, kid_count = %d, desired pos = %u",
               *node_iter, node_iter.number_of_children(), kid_pos))
  return irtree.child(node_iter, kid_pos);
}

// Do nothing
void TREE::Initialize() {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Creating a TREE.Initialize() ... \n"));
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
}

IR_ITER TREE::End() {
  return irtree.end();
}

IRTREE &TREE::Internal_tree() {
  return irtree;
}

UINT32 TREE::Number_of_children(IR_ITER node) {
  return irtree.number_of_children(node);
};

UINT32 TREE::Index(IR_ITER node) {
  return irtree.index(node);
};

UINT32 TREE::Number_of_siblings(IR_ITER node) {
  return irtree.number_of_siblings(node);
}

IR_ITER TREE::Replace_recursive(IR_ITER targ, IR_ITER source) {
  return irtree.move_ontop(targ, source);
}

// Return incremented iterator.
IR_ITER TREE::Remove_node_recursive(IR_ITER pos) {
  if (irtree.number_of_children(pos) > 0) {
    irtree.erase_children(pos);
  }
  return irtree.erase(pos);
}

IR_ITER TREE::Get_parent_in_block(IR_ITER stmt) {
  IR_ITER par = irtree.parent(stmt);
  IR_ITER elem = stmt;
  while (Node(par)->Opcode() != OPC_BLOCK &&
         Node(par)->Opcode() != OPC_FUNC_ENTRY) {
    elem = par;
    par = irtree.parent(par);
  }
  AssertThat(Node(par)->Opcode() == OPC_BLOCK,
             ("couldn't find a block (grand)parent for elem = %d", *par));
  return elem;
};

IR_ITER TREE::Get_parent(IR_ITER expr) {
  IR_ITER par = irtree.parent(expr);
  return par;
};

IR_ITER TREE::Get_parent_region(IR_ITER stmt) {
  IR_ITER par = irtree.parent(stmt);
  // Walk along the parents and find the first if/while stmt.
  // Stop if this is already a function_entry
  while (par != nullptr) {
    if (Node(par)->Opcode() == OPC_WHILE_DO) {
      return par;
    }
    if (Node(par)->Opcode() == OPC_FUNC_ENTRY ||
        irtree.depth(par) <= 1) {
      // error.
      break;
    }
    // Continue on.
    par = irtree.parent(par);
  }
  AssertThat(false, ("Cannot find parent region for node = %d", *stmt));
  return nullptr;
}

IR_ITER TREE::Insert_after(IR_ITER position, IRNODE_IDX node) {
  return irtree.insert_after(position, node);
}

IR_ITER TREE::Insert_before(IR_ITER position, IRNODE_IDX node) {
  return irtree.insert(position, node);
};

///**
// * Utility function for getting the item from the Tree()
// * @param elem
// * @return
// */
//IRNODE &TNode(IR_TREE_ELEM elem) {
//  return *(Tree()->Get_node(elem));
//}

// get the return type part from the opcode.
MTYPE_ID OPCODE_rtype(OPCODE opc) {
  UINT8 type = ((UINT32) opc >> 8) & 0x3f;
  AssertThat(type != MTYPE_UNKNOWN, ("cannot get mtype from opcode = 0x%0x", opc));
  return (MTYPE_ID) type;
}

// get the descriptor type part from the opcode.
MTYPE_ID OPCODE_desc(OPCODE opc) {
  UINT8 type = ((UINT32) opc >> 14) & 0x3f;
  AssertThat(type != MTYPE_UNKNOWN, ("cannot get mtype from opcode = 0x%0x", opc));
  return (MTYPE_ID) type;
}


// get the descriptor type part from the opcode.
OPERATOR OPCODE_operator(OPCODE opc) {
  UINT8 opr = (UINT32) opc & 0xff;
  AssertThat(opr != OPERATOR_UNKNOTREE, ("cannot get opr from opcode = 0x%0x", opc));
  return (OPERATOR) opr;
}

BOOL OPCODE_is_bin_arith(OPCODE opc) {
  OPERATOR opr = OPCODE_operator(opc);
  switch (opr) {
    case OPR_LT:
    case OPR_GT:
    case OPR_ADD:
    case OPR_MPY:
    case OPR_SUB:
    case OPR_DIV:
      return true;
    default:
      return false;
  }
}

BOOL OPCODE_is_const(OPCODE opc) {
  OPERATOR opr = OPCODE_operator(opc);
  switch (opr) {
    case OPR_CONST:
      return true;
    default:
      return false;
  }
}