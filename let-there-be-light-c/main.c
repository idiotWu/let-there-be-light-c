#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "direction.h"
#include "tile.h"
#include "util.h"
#include "list.h"
#include "maze.h"

#define section(msg) printf("\n> %s\n", msg)

typedef struct TestNode {
  int* data;
  struct TestNode* prev;
  struct TestNode* next;
} TestNode;

typedef struct TestList {
  size_t count;
  TestNode* head;
  TestNode* tail;
} TestList;

TestNode* createTestNode(int data) {
  TestNode* node = (TestNode*)createNode();
  int* dp = malloc(sizeof(int));
  
  *dp = data;
  
  node->data = dp;
  
  return node;
}

void listIterator(TestList* list) {
  TestNode* node = list->head;
  
  while (node) {
    printf("%d\t", *node->data);
    
    node = node->next;
  }
  
  putchar('\n');
}

int main(void) {
  section("randomBetween()");
  printf("%lf\n%lf\n", randomBetween(0, 10), randomBetween(0, 10));
  
  section("randomInt()");
  printf("%d\n", randomInt(0, 1));
  printf("%d\n", randomInt(10, 1));
  
  section("swap() test");
  int a = -1;
  int b = 2;
  swap(&a, &b, sizeof(int));
  
  printf("a=%d b=%d\n", a, b);
  
  section("shuffle()");
  int arr[10];
  
  for (size_t i = 0; i < 10; i++) {
    arr[i] = (int)i;
  }
  
  shuffle(arr, 10, sizeof(arr[0]));
  
  for (size_t i = 0; i < 10; i++) {
    printf("%d\t", arr[i]);
  }
  
  putchar('\n');
  
  section("generic linked list");
  
  TestList* list = (TestList*)createList();
  
  TestNode* n0 = createTestNode(0);
  TestNode* n1 = createTestNode(1);
  TestNode* n2 = createTestNode(2);
  
  SAFE_DUCK_LIST(TestList);
  SAFE_DUCK_NODE(TestNode);

  section("listAppend()");
  listAppend((List*)list, (Node*)n0);
  assert(list->count == 1);
  assert(list->head == n0);
  assert(list->tail == n0);
  listIterator(list);
  
  section("listPrepend()");
  listPrepend((List*)list, (Node*)n1);
  assert(list->count == 2);
  assert(list->head == n1);
  assert(list->tail == n0);
  assert(n1->next == n0);
  assert(n0->prev == n1);
  listIterator(list);
  
  section("listInsertAfter()");
  listInsertAfter((List*)list, (Node*)n1, (Node*)n2);
  assert(list->count == 3);
  assert(list->head == n1);
  assert(list->tail == n0);
  assert(n1->next == n2);
  assert(n2->prev == n1);
  assert(n2->next == n0);
  assert(n0->prev == n2);
  listIterator(list);
  
  section("listDelete()");
  listDelete((List*)list, (Node*)n2);
  assert(list->count == 2);
  assert(list->head == n1);
  assert(list->tail == n0);
  assert(n1->next == n0);
  assert(n0->prev == n1);
  listIterator(list);
  
  listDelete((List*)list, (Node*)n0);
  assert(list->count == 1);
  assert(list->head == n1);
  assert(list->tail == n1);
  listIterator(list);
  
  listDelete((List*)list, (Node*)n1);
  assert(list->count == 0);
  assert(list->head == NULL);
  assert(list->tail == NULL);
  listIterator(list);
  
  listDestory((List*)list);
  
  section("initMaze()");
  Tile tiles[MAP_SIZE][MAP_SIZE];
  
  size_t pathLength = initMaze(4, 8, 16, tiles);
  
  printf("path length: %lu\n", pathLength);
  
  for (size_t i = 0; i < MAP_SIZE; i++) {
    for (size_t j = 0; j < MAP_SIZE; j++) {
      printf("%2c", tiles[i][j] == TILE_WALL ? ' ' : '*');
    }
    
    putchar('\n');
  }
    
  return 0;
}
