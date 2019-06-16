#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

void listAppend(List* list, Node* node) {
  if (list->count == 0) {
    list->head = list->tail = node;
  } else {
    list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
  }

  list->count++;
}

void listPrepend(List* list, Node* node) {
  if (list->count == 0) {
    list->head = list->tail = node;
  } else {
    list->head->prev = node;
    node->next = list->head;
    list->head = node;
  }

  list->count++;
}

void listInsertAfter(List* list, Node* prev, Node* node) {
  Node* next = prev->next;

  prev->next = node;
  node->prev = prev;

  next->prev = node;
  node->next = next;

  list->count++;
}

void listDelete(List* list, Node* node) {
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

void listCopy(List* dest, List* src, size_t dataSize) {
  Node* snode = src->head;

  while (snode) {
    Node* dnode = createNode();
    dnode->data = malloc(dataSize);

    memcpy(dnode->data, snode->data, dataSize);

    listAppend(dest, dnode);

    snode = snode->next;
  }
}

List* listClone(List* src, size_t dataSize) {
  List* dest = createList();

  listCopy(dest, src, dataSize);

  return dest;
}

void listFreeNode(Node* node) {
  free(node->data);
  free(node);
}

void listDestory(List* list) {
  Node* node = list->head;

  while (node) {
    listFreeNode(node);
    node = node->next;
  }

  free(list);
}

List* createList(void) {
  List* list = malloc(sizeof(List));

  list->count = 0;
  list->head = list->tail = NULL;

  return list;
}

Node* createNode(void) {
  Node* node = malloc(sizeof(Node));

  node->data = NULL;
  node->prev = node->next = NULL;

  return node;
}
