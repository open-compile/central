/* bug03.c - minimal reproducer for the fall-through edge bug in
   opt_cfg_builder.cxx.

   Root cause: Build_function_body omits the branch_bb -> fall-through
   edge after TRUEBR/FALSEBR.  Without it, the fall-through BB is
   unreachable, Compute_rpo's DFS skips it, Compute_idom never runs,
   and Print_dom_tree / Print_pretty hit
   "idom is NULL. DOM is not built?" at cfg_common.h:274.

   Reproduce (against an unfixed build):
       ./build/install/bin/compiler testcase/bugs/bug03.c --target=native -c --verbose
   Expected (unfixed): "### Assertion Failure in Phase LOWER: idom is NULL".
   Expected (fixed):   object file generated cleanly, no assertion. */

void f(int a) {
    if (a > 0)
        a = 1;
}
