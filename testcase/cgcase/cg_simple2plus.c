int foo1(int a, int b) {
  int x = 3;
  x = a + b;
  x = x * b;
  a = a - b;
  b = (x + b) * a;
  b = (x * a) * (b + x) * x;
  x = x * b;
  return x > 30; 
}

// the x > 30 is illegal in sysy lang, and compiler fails at assertion.
