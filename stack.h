#include <error.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct stack{
  int size;
  struct stack* next;
  void* data;
} stack;

void exception(int index);
void stack_init(stack* s);
void* stack_top(stack* s);
void stack_push(stack* s, void* data);
void stack_pop(stack* s, int err);
int stack_size(stack* s);
void* stack_data(stack* s, int index);
void stack_change(stack* s, int index, void* data);
void stack_delete(stack* s, int index);