/* bug02_complex.c - complex reproducer suite for the fall-through
   edge bug in opt_cfg_builder.cxx.  Each function isolates a
   different control-flow shape that stresses the same root cause:
   Build_function_body omits the branch_bb -> fall-through edge
   after TRUEBR/FALSEBR, so Compute_rpo's DFS misses whole BBs and
   Compute_idom's iteration never runs, eventually asserting
   "idom is NULL. DOM is not built?" at cfg_common.h:274.

   The original simple reproducer lives in bug03.c.

   Reproduce (against an unfixed build):
       ./build/install/bin/compiler testcase/bugs/bug02_complex.c --target=native -c --verbose
   Expected (unfixed): "### Assertion Failure in Phase LOWER: idom is NULL"
                        at the first function that contains a branch.
   Expected (fixed):   all five functions compile cleanly. */

/* 1. nested if/else: two idom levels deep. */
int nested(int a, int b) {
    int r = 0;
    if (a > 0) {
        if (b > 0)
            r = 1;
        else
            r = 2;
    } else {
        r = 3;
    }
    return r;
}

/* 2. multi-way branch: if / else if / else chain. */
int chain(int a) {
    if (a > 10)
        return 1;
    else if (a > 5)
        return 2;
    else if (a > 0)
        return 3;
    else
        return 4;
}

/* 3. empty if body: only branch, no work inside. */
int empty_then(int a) {
    if (a > 0) { }
    return 1;
}

/* 4. consecutive branches: no fall-through code between them. */
int consecutive(int a, int b) {
    if (a > 0)
        a = 1;
    if (b > 0)
        b = 1;
    return a + b;
}

/* 5. branch with explicit return right after: tightens the
      missing edge because there is exactly one successor BB. */
int branch_then_return(int a) {
    if (a > 0)
        a = a + 1;
    return a;
}
