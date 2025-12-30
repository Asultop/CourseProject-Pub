#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../src-extends/passwordInputSimulator.h"
int main() {
    char pwd[PWDLEN + 1];

    printf("Input password (← → backspace supported): ");
    fflush(stdout);
    getpwd(pwd, PWDLEN);

    printf("Your password: %s\n", pwd);
    return 0;
}