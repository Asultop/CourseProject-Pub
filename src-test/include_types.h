#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

// 多行 struct 定义
struct Person {
    char name[64];
    int age;
};

// 多行匿名 typedef
typedef struct {
    int id;
    char desc[128];
} Item;

#endif // INCLUDE_TYPES_H
