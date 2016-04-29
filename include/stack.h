#ifndef _STACK_H
#define _STACK_H

#include "Board.h"

typedef struct stackCDT *stackADT;
typedef struct {
  BoardADT Board;
  int Move;
} stackElementT;

stackADT EmptyStack(void);
void Push(stackADT, stackElementT);
stackElementT Pop(stackADT);
int StackDepth(stackADT);
int StackIsEmpty(stackADT);
void DestroyStack(stackADT);

#endif