#include "stack.h"

void exception(int index){
  fprintf(stderr, "Exception encountered at line %d\n", index);
  exit(1);
}
void stack_init(stack* s){
  s->size = 0;
  s->next = NULL;
  s->data = NULL;
  return;
}
void* stack_top(stack* s){
  stack* cur = s->next;
  int i;
  for (i = 0; i<s->size-1; i++)
    cur = cur->next;
  return cur->data;
}
void stack_push(stack* s, void* data){
  stack* cur = s;
  int i;
  for (i = 0; i<s->size; i++)
    cur = cur->next;
  cur->next = (stack *) malloc (sizeof(stack));
  stack_init(cur->next);
  cur->next->data = data;
  s->size++;
  return;
}
void stack_pop(stack* s, int err){
  stack* cur = s->next;
  if (s->size <= 0)
    exception(err);
  int i;
  for (i = 0; i<(s->size)-1; i++){
    cur = cur->next;}
  free(cur->next);
  cur->next = NULL;
  s->size--;
  return;
}
int stack_size(stack* s){return s->size;}
void* stack_data(stack* s, int index){
  stack* cur = s->next;
  int i;
  if (index>=s->size)
    exception(index);
  for (i = 0; i<index; i++)
    cur = cur->next;
  return cur->data;
}
void stack_change(stack* s, int index, void* data){
  stack* cur = s->next;
  int i;
  if (index>=s->size)
    exception(index);
  for (i = 0; i<index; i++)
    cur = cur->next;
  free(cur->data);
  cur->data = data;
}
void stack_delete(stack* s, int index){
  stack* cur = s;
  int i;
  if (index>=s->size)
    exception(index);
  for (i = 0; i<index; i++)
    cur = cur->next;
  free(cur->next->data);
  stack* temp = cur->next->next;
  free(cur->next);
  cur->next = temp;
  s->size--;
  return;
}
