/**
 * @file
 * @brief 双方向連結リスト
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "util/list.h"

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

bool listFindDelete(List* list, void* data) {
  ListIterator it = createListIterator(list);

  while (!it.done) {
    Node* node = it.next(&it);

    if (node->data == data) {
      listDelete(list, node);
      return true;
    }
  }

  return false;
}

void listFreeNode(Node* node) {
  free(node->data);
  free(node);
}

void listClear(List* list) {
  ListIterator it = createListIterator(list);

  while (!it.done) {
    Node* node = it.next(&it);
    listDelete(list, node);
  }
}

void listDestory(List* list) {
  listClear(list);
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

///////////// LIST ITERATOR /////////////

/**
 * @brief イテレーションが終わったのかをチェックする
 *
 * @param it 目標のイテレータ
 *
 * @return true  イテレーションが終わった
 * @return false イテレーションが進行中
 */
static bool iteratorFinished(ListIterator* it) {
  return it->nextNode == NULL || it->nextIndex >= it->initialLength;
}

/**
 * @brief イテレータを次のノードに移動させる
 *
 * @details `iterator.next(list)` を呼び出すたびに，イテレータを次のノードに移動させる
 *
 * @param it イテレータ自身（self）
 *
 * @return Node* 今回のイテレーションにおいて，イテレータが指すノード
 */
static Node* iterateNext(ListIterator* it) {
  if (it->done) {
    return NULL;
  }

  Node* currentNode = it->nextNode;

  it->nextNode = currentNode->next;
  it->nextIndex++;

  it->done = iteratorFinished(it);

  return currentNode;
}

ListIterator createListIterator(List* list) {
  ListIterator it;

  it.list = list;
  it.nextNode = list->head;
  it.initialLength = list->count;
  it.nextIndex = 0;
  it.done = iteratorFinished(&it);

  it.next = iterateNext;

  return it;
}
