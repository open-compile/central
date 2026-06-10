// ============================================================================
// CLAUDE-MARKER  STATUS: 保留 (testcase, 不需要重写)
// ============================================================================
// DCE + Const Prop 优化测试
// 期望：
//   - DCE 删除无 side-effect 的死代码
//   - CPROP 把 x = 1 + 2 折叠成 x = 3
//   - 条件常量被折叠为 TRUEBR/FALSEBR 死分支
int g;

int dead_var() {
  int x;
  int y;
  int z;
  x = 1;          // 常量
  y = 2;          // 常量，定义后未使用 -> DCE 删
  z = 100;        // 定义后未使用 -> DCE 删
  g = x + 3;      // x+3 = 4 -> CPROP 折叠
  return g;
}

int cprop_chain() {
  int a;
  int b;
  int c;
  a = 10;          // 常量
  b = a + 5;       // a+5 = 15
  c = b * 2;       // b*2 = 30
  if (c > 100) {   // 30 > 100 永假，整个 if 体可被 DCE
    g = 999;
  }
  return c;
}

int branch_fold() {
  int k;
  k = 1;           // 条件总是 true
  if (k) {
    return 7;
  }
  return 8;        // 死代码
}

int main() {
  dead_var();
  cprop_chain();
  return branch_fold();
}
