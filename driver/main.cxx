#include <stdio.h>
#include "../common/basic.h"

int main(int argc, char **argv, char **envp) {
    printf("Usage: occ [source code] [options] \n");
    AssertThat(1 == 2, ("1 is not equal to 2"));
    return 0;
}