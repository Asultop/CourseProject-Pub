//https://www.acwing.com/problem/content/3485/
//2025307070614 陈鹏屹
//2025307070630 沈旭燃
//2025307070643 赵哲
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#define MAX_LEN 802

void add(const char* num1, const char* num2, char* result){
    int len1=strlen(num1);
    int len2=strlen(num2);
    int carry=0,i=0;
    char rev_rst[MAX_LEN];
    while(len1>0 || len2>0 || carry>0){
        int digit1=(len1>0)?(num1[--len1]-'0'):0;
        int digit2=(len2>0)?(num2[--len2]-'0'):0;
        int sum=digit1+digit2+carry;
        rev_rst[i++]=(sum%10)+'0';
        carry=sum/10;
    }
    rev_rst[i]='\0';
    if(i==0){
        strcpy(result,"0");
        return;
}
    int k=0;
    while(i>0){
        result[k++]=rev_rst[--i];
    }
    result[k]='\0';
}

void jian(const char* num1, const char* num2, char* result){
    int len1=strlen(num1);
    int len2=strlen(num2);
    int borrow=0,i=0;
    char rev_rst[MAX_LEN];
    while(len1>0){
        int digit1=num1[--len1]-'0';
        int digit2=(len2>0)?(num2[--len2]-'0'):0;
        int diff=digit1-digit2-borrow;
        if(diff<0){
            diff+=10;
            borrow=1;
        }else{
            borrow=0;
        }
        rev_rst[i++]=diff+'0';
    }
    rev_rst[i]='\0';
    int k=0;
    while(i>0){
        result[k++]=rev_rst[--i];
    }
    result[k]='\0';
    int start_index=0;
    while(result[start_index]=='0' && result[start_index+1]!='\0'){
        start_index++;
    }
    int m=0;
    for(;start_index<strlen(result);start_index++){
        result[m++]=result[start_index];
    }
    result[m]='\0';
}

void multiply(const char* num1, const char* num2, char* result){
    if(strcmp(num1,"0")==0 || strcmp(num2,"0")==0){
        strcpy(result,"0");
        return;
    }
    int len1=strlen(num1);
    int len2=strlen(num2);
    int res_len=len1+len2;
    int temp[res_len];
    for(int i=0;i<res_len;i++) temp[i]=0;
    for(int i=len1-1;i>=0;i--){
        for(int j=len2-1;j>=0;j--){
            int product=(num1[i]-'0')*(num2[j]-'0');
            int sum=product+temp[i+j+1];
            temp[i+j+1]=sum%10;
            temp[i+j]+=sum/10;
        }
    }
    int start_idx=0;
    while(start_idx<res_len && temp[start_idx]==0){
        start_idx++;
    }
    int i=0;
    for(;start_idx<res_len;start_idx++){
        result[i++]=temp[start_idx]+'0';
    }
    result[i]='\0';
}

int compare_abs(const char* num1, const char* num2) {
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    if (len1 != len2) {
        return (len1 > len2) ? 1 : -1;
    }
    int cmp = strcmp(num1, num2);;
    if (cmp > 0) return 1;
    else if (cmp < 0) return -1;
    return 0;
}
int main() {
    char a[MAX_LEN], b[MAX_LEN];
    scanf("%s %s", a, b);

    
    bool is_a_neg = (a[0] == '-');
    bool is_b_neg = (b[0] == '-');
    const char* a_num = is_a_neg ? a + 1 : a;
    const char* b_num = is_b_neg ? b + 1 : b;

    char result[MAX_LEN];

    
    // 计算 a + b 
    if (is_a_neg == is_b_neg) {
        // 情况1: (+) + (+) 或 (-) + (-)
        // 两种情况都归结为绝对值相加。
        add(a_num, b_num, result);
        if (is_a_neg) { // 如果两个都是负数，则结果也为负
            printf("-%s\n", result);
        } else {
            printf("%s\n", result);
        }
    } else {
        // 情况2: 一个为正，一个为负 (结果相当于较大绝对值减去较小绝对值)
        int cmp = compare_abs(a_num, b_num);
        if (cmp > 0) { // |a| > |b|
            jian(a_num, b_num, result);
            if (is_a_neg) { // -|a| + |b|, 结果为负
                printf("-%s\n", result);
            } else { // |a| + (-|b|), 结果为正
                printf("%s\n", result);
            }
        } else if (cmp < 0) { // |a| < |b|
            jian(b_num, a_num, result);
            if (is_b_neg) { // |a| + (-|b|), 结果为负
                printf("-%s\n", result);
            } else { // -|a| + |b|, 结果为正
                printf("%s\n", result);
            }
        } else { // |a| == |b|, 结果是 0
            printf("0\n");
        }
    }

    //  计算 a - b 
    if (!is_a_neg && !is_b_neg) {
        // 情况1: (+) - (+) -> a - b
        int cmp = compare_abs(a_num, b_num);
        if (cmp >= 0) { // a >= b
            jian(a_num, b_num, result);
            printf("%s\n", result);
        } else { // a < b
            jian(b_num, a_num, result);
            printf("-%s\n", result);
        }
    } else if (!is_a_neg && is_b_neg) {
        // 情况2: (+) - (-) -> a - (-b) -> a + b
        add(a_num, b_num, result);
        printf("%s\n", result);
    } else if (is_a_neg && !is_b_neg) {
        // 情况3: (-) - (+) -> -a - b -> -(a + b)
        add(a_num, b_num, result);
        printf("-%s\n", result);
    } else { // is_a_neg && is_b_neg
        // 情况4: (-) - (-) -> -a - (-b) -> b - a
        int cmp = compare_abs(b_num, a_num);
        if (cmp >= 0) { // b >= a
            jian(b_num, a_num, result);
            printf("%s\n", result);
        } else { // b < a
            jian(a_num, b_num, result);
            printf("-%s\n", result);
        }
    }

    // 计算 a * b 
    multiply(a_num, b_num, result);
    // 如果两个数符号不同且结果不为 "0"，则输出负号
    if (is_a_neg != is_b_neg && strcmp(result, "0") != 0) {
        printf("-%s\n", result);
    } else {
        printf("%s\n", result);
    }

    return 0;
}
