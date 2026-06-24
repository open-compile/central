/* bug02.c - reproduce: opt_cfg_builder omits the fall-through edge
   after TRUEBR/FALSEBR, causing DOM_BUILDER to assert.

   Trigger: any function body with a conditional branch (if / else).
   Root cause: CFG_FROM_IR::Build_function_body calls End_bb() to
   create a fall-through BB after a branch, but never adds the
   branch_bb -> fall_through edge.  As a result, Compute_rpo's DFS
   only sees the entry BB, Compute_idom's iteration never runs, and
   downstream Print_dom_tree / Print_pretty hit
   "idom is NULL. DOM is not built?" at cfg_common.h:274.

   Reproduce (against an unfixed build):
       ./build/install/bin/compiler testcase/bugs/bug02.c --target=native -c --verbose
   Expected (unfixed): "### Assertion Failure in Phase LOWER: idom is NULL".
   Expected (fixed):   object file generated cleanly, no assertion. */

int bug_if_only(int a) {
    int x = 0;
    if (a > 0) {
        x = 1;
    } else {
        x = 2;
    }
    return x;
}

int bug_early_return(int a) {
    if (a > 0) {
        return 1;
    }
    return 2;
}
