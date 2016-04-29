#include <windows.h>
#include "list.h"


struct listCDT {
  listElementT head;
  listADT tail;
};


listADT EmptyList()
{
  return((listADT) NULL);
}


listADT ListCons(listElementT head, listADT tail)
{
  listADT list;

  list = (listADT) GlobalAlloc(GPTR, sizeof(*list));
  list->head = head;
  list->tail = tail;
  return (list);
}


listElementT ListHead(listADT list)
{
  if (list == (listADT) NULL) exit(0);
  return (list->head);
}


listADT ListTail(listADT list)
{
  if (list == (listADT) NULL) exit(0);
  return (list->tail);
}


int ListEqual(listADT list1, listADT list2)
{
  if (list1 == (listADT) NULL && list2 == (listADT) NULL)
    return 1;
  else if (list1 != (listADT) NULL && list2 != (listADT) NULL)
    return (ListHead(list1) == ListHead(list2) && ListEqual(ListTail(list1), ListTail(list2)));
  else
    return 0;
}


int ListLength(listADT list)
{
  if (ListEqual(list, EmptyList())) return 0;
  else
    return (1 + ListLength(ListTail(list)));
}


// Free the memory of a list
void DestroyList(listADT list)
{
  if (ListEqual(list, EmptyList()))
    return;
  DestroyList(ListTail(list));
  GlobalFree((HANDLE) list);
}