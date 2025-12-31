//https://www.acwing.com/problem/content/1/
//2025307070630 沈旭燃
//2025307070614 陈鹏屹
//2025307070643 赵哲
#include <stdio.h>
int main() {
    int n;
    int count = 0;
    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        int num;
        scanf("%d", &num);
        if (num != 1) {
            count++;
        }
    }
    printf("%d\n", count);
    return 0;
}