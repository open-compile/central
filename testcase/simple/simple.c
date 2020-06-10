// Machine TYPE
// I integer, I1, I2, I4, I8
// U unsigned int, U1, U2, U4, U8
// B bool
// V void
// A Array

void foo() {
    int a = 3;
}

/*
// ST0 a, ty=#{int}
FUNCENTRY foo
  BLOCK
      INTCONST 3
    STID, a
  ENDBLOCK
*/

void foo2() {
    int a = 3 + 4;
}

/*
// ST0 a, ty=#{int}

FUNCENTRY foo2 
  BLOCK
        INTCONST 3
        INTCONST 4
      I4ADD
    STID, a
  ENDBLOCK
*/
  
int bar(int m) {
    ///...
}

void foo3 () {
    int a;
    a = bar(a) + bar(a);
}

/*

      LDID a
    PARAMS
  CALL bar

    LDID  PREG(ret_val)
  STID PREG(2)

      LDID a
    PARAMS
  CALL bar

    LDID  PREG(ret_val)
  STID PREG(3)

      LDID PREG(2)
      LDID PREG(3)
    ADD 
  STID a
  */

int foo4(int m) {
    if (m <= 2) return 1;
    return foo4(m - 1) + foo4(m - 2);
}

/*
        LDID m
        I4CONST 1
      SUB
    PARAMS
  CALL foo3
    LDID  PREG(ret_val)
  STID PREG(3) -> stack
*/