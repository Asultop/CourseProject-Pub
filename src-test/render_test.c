#include "include_types.h"

// 多行 typedef 示例
typedef struct {
    double a;
    char b[32];
} ComplexType;

using Alias = ComplexType;

int main(void) {
    ComplexType v;
    Alias a;
    struct Person p;
    Item it;
    (void)v; (void)a; (void)p; (void)it;
    return 0;
}
