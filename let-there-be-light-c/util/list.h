#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef struct Node {
  void* data;
  struct Node* prev;
  struct Node* next;
} Node;

typedef struct List {
  size_t count;
  Node* head;
  Node* tail;
} List;

void listAppend(List* list, Node* node);
void listPrepend(List* list, Node* node);

void listInsertAfter(List* list, Node* prev, Node* node);
void listDelete(List* list, Node* node);

void listCopy(List* dest, List* src, size_t dataSize);
List* listClone(List* src, size_t dataSize);

bool listFindDelete(List* list, void* data);

void listFreeNode(Node* node);
void listClear(List* list);
void listDestory(List* list);

List* createList(void);
Node* createNode(void);

///////////// LIST ITERATOR /////////////

typedef struct ListIterator {
  List* list;
  Node* nextNode;

  size_t initialLength;
  size_t nextIndex;

  bool done;

  Node* (*next)(struct ListIterator*);
} ListIterator;

ListIterator createListIterator(List* list);


#endif
