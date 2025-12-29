#include "md5.h"
#include <stdio.h>
int main(){
    char md5_hex[33];
    MD5_String("Hello, World!", md5_hex);
    printf("MD5(\"Hello, World!\") = %s\n", md5_hex);
    return 0;
}