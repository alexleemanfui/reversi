#ifndef _LIST_H
#define _LIST_H

typedef struct listCDT *listADT;
typedef int listElementT;

listADT EmptyList(void);
listADT ListCons(listElementT, listADT);
listElementT ListHead(listADT);
listADT ListTail(listADT);
int ListEqual(listADT, listADT);
int ListLength(listADT);
void DestroyList(listADT);

#endif