#ifndef LIST_H
#define LIST_H

#include <stddef.h>
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

void listAppend(List* list, Node* node);
void listPrepend(List* list, Node* node);

void listInsertAfter(List* list, Node* prev, Node* node);
void listDelete(List* list, Node* node);

void listCopy(List* dest, List* src, size_t bytesPerElement);
List* listClone(List* src, size_t dataSize);

void listFreeNode(Node* node);
void listDestory(List* list);

List* createList(void);
Node* createNode(void);

#endif
