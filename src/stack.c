#include <windows.h>
#include "stack.h"


struct stackCDT {
  stackElementT elements[61];
  int count;
};


stackADT EmptyStack(void)
{
  stackADT stack;

  stack = (stackADT) GlobalAlloc(GPTR, sizeof(*stack));
  stack->count=0;
  return (stack);
}


void Push(stackADT stack, stackElementT element)
{
  stack->elements[(stack->count)++] = element;
}


stackElementT Pop(stackADT stack)
{
  return (stack->elements[--(stack->count)]);
}


int StackDepth(stackADT stack)
{
  return (stack->count);
}


int StackIsEmpty(stackADT stack)
{
  return (stack->count == 0);
}


// Free the memory of a stack
void DestroyStack(stackADT stack)
{
  stackElementT stackElement;

  // Destroy all the board in the stack
  while (!StackIsEmpty(stack))
  {
    stackElement = Pop(stack);
    DestroyBoard((HANDLE) stackElement.Board);
  }

  // Destroy the whole stack
  GlobalFree((HANDLE) stack);
}