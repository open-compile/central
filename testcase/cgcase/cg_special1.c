// #include <stdio.h>

/*
 * Two-diamond branch test case for code generation / register allocation.
 * Block labels (B1..B7) map 1:1 to the CFG.
 *
 *   a, t      : long live ranges spanning the branches
 *   x, y      : local to B2  | never live at the same time as p, q
 *   p, q      : local to B3  | (B2 and B3 are mutually exclusive)
 *   r         : defined in BOTH B2 and B3, merged/used at B4
 *   w         : defined in BOTH B5 and B6, merged/used at B7
 */
int cg_test(int param1, int param2) {
    int a = param1;                 /* B1 */
    int b = param2;
    int t = a + b;

    int r;
    if (t < 10) {                   /* B2 */
        int x = a * t;
        int y = x + 1;
        r = y - b;
    } else {                        /* B3 */
        int p = b * t;
        int q = p + a;
        r = q + 1;
    }

    int z = r + t;                  /* B4 */

    int w;
    if (z > 100) {                  /* B5 */
        w = z * 2;
    } else {                        /* B6 */
        w = z + a;
    }

    return w;                       /* B7 */
}

// int main(void) {
//     printf("%d\n", cg_test(2, 3));    /* t=5  -> B2, then z=13  -> B6 */
//     printf("%d\n", cg_test(20, 30));  /* t=50 -> B3, then z>100 -> B5 */
//     return 0;
// }