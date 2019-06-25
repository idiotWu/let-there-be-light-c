#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#define __HAS_ELEMENT(TYPE_A, TYPE_B, ELEMENT) \
  assert(offsetof(TYPE_A, ELEMENT) == offsetof(TYPE_B, ELEMENT))

// duck typing check
#define SAFE_DUCK_LIST(DuckList) do {\
  __HAS_ELEMENT(DuckList, List, count);\
  __HAS_ELEMENT(DuckList, List, head);\
  __HAS_ELEMENT(DuckList, List, tail);\
} while(0)

#define SAFE_DUCK_NODE(DuckNode) do {\
  __HAS_ELEMENT(DuckNode, Node, data);\
  __HAS_ELEMENT(DuckNode, Node, prev);\
  __HAS_ELEMENT(DuckNode, Node, next);\
} while(0)

#define defNode(NAME, DATA_TYPE) \
  typedef struct NAME {\
    DATA_TYPE* data;\
    struct NAME* prev;\
    struct NAME* next;\
  } NAME

#define defList(NAME, NODE_TYPE) \
  typedef struct NAME {\
    size_t count;\
    NODE_TYPE* head;\
    NODE_TYPE* tail;\
  } NAME

defNode(Node, void);
//typedef struct Node {
//  void* data;
//  struct Node* prev;
//  struct Node* next;
//} Node;

defList(List, Node);
//typedef struct List {
//  size_t count;
//  Node* head;
//  Node* tail;
//} List;

void listAppend(void* list, void* node);
void listPrepend(void* list, void* node);

void listInsertAfter(void* list, void* prev, void* node);
void listDelete(void* list, void* node);

void listCopy(void* dest, void* src, size_t dataSize);
void* listClone(void* src, size_t dataSize);

bool listFindDelete(void* list, void* data);

void listFreeNode(void* node);
void listClear(void* _list);
void listDestory(void* list);

void* createList(void);
void* createNode(void);

///////////// LIST ITERATOR /////////////

typedef struct ListIterator {
  List* list;
  Node* nextNode;

  size_t initialLength;
  size_t nextIndex;

  bool done;

  void* (*next)(struct ListIterator*);
} ListIterator;

ListIterator createListIterator(void* list);


#endif
