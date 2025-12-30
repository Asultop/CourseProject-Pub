#include "../src-extends/stack.h"
#include <stdio.h>
int main(){
    stack_t myStack;
    initialize_stack(&myStack);

    push(&myStack, 10);
    push(&myStack, 20);
    push(&myStack, 30);

    printf("Top : %d\n", peek(&myStack));
    display_stack(&myStack);
    printf("Popped: %d\n", pop(&myStack));
    printf("Top -> pop is: %d\n", peek(&myStack));
    display_stack(&myStack);
    return 0;
}