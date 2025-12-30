
#include <stdio.h>
#include "stack.h"

int is_stack_empty(stack_t *stack){
	return stack->top == -1;
}

int is_stack_full(stack_t *stack){
	return stack->top == STACK_SIZE - 1;
}

int initialize_stack(stack_t *stack){
	int retval = -1;

	do {
		if (stack == NULL) {
			retval = STACK_ERR;
			break;
		} else {
			stack->top = -1;
			retval = STACK_OK;
			break;
		}
	} while(0);

	return retval;
}
int push(stack_t *stack, int value){
	int retval = -1;

	do {
		if (is_stack_full(stack)) {
			retval = STACK_FULL;
			break;
		} else {
			stack->element[++stack->top] = value;
			retval = STACK_OK;
			break;
		}
	} while(0);

	return retval;
}
int pop(stack_t *stack){
	int retval = -1;

	do {
		if (is_stack_empty(stack)) {
			retval = STACK_EMPTY;
			break;
		} else {
			retval = stack->element[stack->top--];
			break;
		}
	} while(0);

	return retval;
}
int peek(stack_t *stack){
	int retval = -1;

	do {
		if (is_stack_empty(stack)) {
			retval = STACK_EMPTY;
			break;
		} else {
			retval = stack->element[stack->top];
			break;
		}
	} while(0);

	return retval;
}


int display_stack(stack_t *stack){
	int i = 0;
	int retval = -1;

	do {
		if (is_stack_empty(stack)) {
			retval = STACK_EMPTY;
			break;
		} else {
			printf("Stack: { ");

			for (i = 0; i <= stack->top -1; i++) {
				printf("%d, ", stack->element[i]);
			}
            printf("%d }", stack->element[stack->top]);
			printf("\n");
			retval = STACK_OK;
			break;
		}
	} while(0);

	return retval;
}
