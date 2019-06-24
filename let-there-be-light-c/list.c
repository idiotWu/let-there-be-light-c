#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "list.h"

void listAppend(void* _list, void* _node) {
  List* list = _list;
  Node* node = _node;

  if (list->count == 0) {
    list->head = list->tail = node;
  } else {
    list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
  }

  list->count++;
}

void listPrepend(void* _list, void* _node) {
  List* list = _list;
  Node* node = _node;
  
  if (list->count == 0) {
    list->head = list->tail = node;
  } else {
    list->head->prev = node;
    node->next = list->head;
    list->head = node;
  }

  list->count++;
}

void listInsertAfter(void* _list, void* _prev, void* _node) {
  List* list = _list;
  Node* prev = _prev;
  Node* node = _node;

  Node* next = prev->next;

  prev->next = node;
  node->prev = prev;

  next->prev = node;
  node->next = next;

  list->count++;
}

void listDelete(void* _list, void* _node) {
  List* list = _list;
  Node* node = _node;

  assert(list->count > 0);

  Node* prev = node->prev;
  Node* next = node->next;

  if (list->head == node) {
    list->head = next;
  }
  if (list->tail == node) {
    list->tail = prev;
  }

  if (prev != NULL) {
    prev->next = next;
  }
  if (next != NULL) {
    next->prev = prev;
  }

  node->next = node->prev = NULL;

  listFreeNode(node);

  list->count--;
}

void listCopy(void* _dest, void* _src, size_t dataSize) {
  List* dest = _dest;
  List* src = _src;

  Node* snode = src->head;

  while (snode) {
    Node* dnode = createNode();
    dnode->data = malloc(dataSize);

    memcpy(dnode->data, snode->data, dataSize);

    listAppend(dest, dnode);

    snode = snode->next;
  }
}

void* listClone(void* _src, size_t dataSize) {
  List* src = _src;

  List* dest = createList();

  listCopy(dest, src, dataSize);

  return dest;
}

void listFreeNode(void* _node) {
  Node* node = _node;

  free(node->data);
  free(node);
}

void listClear(void* _list) {
  List* list = _list;

  ListIterator it = createListIterator(list);

  while (!it.done) {
    Node* node = it.next(&it);
    listDelete(list, node);
  }
}

void listDestory(void* _list) {
  listClear(_list);
  free(_list);
}

void* createList(void) {
  List* list = malloc(sizeof(List));

  list->count = 0;
  list->head = list->tail = NULL;

  return list;
}

void* createNode(void) {
  Node* node = malloc(sizeof(Node));

  node->data = NULL;
  node->prev = node->next = NULL;

  return node;
}

///////////// LIST ITERATOR /////////////

static bool iteratorFinished(ListIterator* it) {
  return it->nextNode == NULL || it->nextIndex >= it->initialLength;
}

static void* iterateNext(ListIterator* it) {
  if (it->done) {
    return NULL;
  }

  Node* currentNode = it->nextNode;

  it->nextNode = currentNode->next;
  it->nextIndex++;

  it->done = iteratorFinished(it);

  return currentNode;
}

ListIterator createListIterator(void* _list) {
  List* list = _list;
  ListIterator it;

  it.list = list;
  it.nextNode = list->head;
  it.initialLength = list->count;
  it.nextIndex = 0;
  it.done = iteratorFinished(&it);

  it.next = iterateNext;

  return it;
}
