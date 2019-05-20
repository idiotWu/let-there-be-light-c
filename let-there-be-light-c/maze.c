#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "maze.h"
#include "tile.h"
#include "util.h"
#include "list.h"
#include "config.h"
#include "direction.h"
#include "floodfill.h"

typedef struct Spawner {
  size_t x;
  size_t y;
} Spawner;

typedef struct MazeBuilder {
  size_t x;
  size_t y;
  int vx;
  int vy;
  size_t limit;
  size_t remain;
  size_t minDistance;
  size_t maxDistance;
  Direction direction;
  Spawner* spawners;
  size_t spawnerIndex;
  size_t spawnerCount;
} MazeBuilder;

defNode(BuilderNode, MazeBuilder);
defList(BuilderList, BuilderNode);

static Direction filterDirections(size_t x,
                                  size_t y,
                                  size_t limit,
                                  Direction candidates) {
  if (x == 0 || x == 1 /* odd numbers of distance are not allowed */) {
    candidates &= ~DIR_LEFT; // clear the specific bit
  } else if (x == limit || x == limit - 1) {
    candidates &= ~DIR_RIGHT;
  }

  if (y == 0 || y == 1) {
    candidates &= ~DIR_UP;
  } else if (y == limit || y == limit - 1) {
    candidates &= ~DIR_DOWN;
  }

  return candidates;
}

// ============ Builder Begin ============ //

static void updateDistance(MazeBuilder* builder) {
  size_t distance = randomInt((int)builder->minDistance, (int)builder->maxDistance);

  assert(distance != 0);

  switch (builder->direction) {
    case DIR_UP:
      builder->vx = 0;
      builder->vy = -1;
      break;

    case DIR_DOWN:
      builder->vx = 0;
      builder->vy = 1;
      break;

    case DIR_LEFT:
      builder->vx = -1;
      builder->vy = 0;
      break;

    case DIR_RIGHT:
      builder->vx = 1;
      builder->vy = 0;
      break;

    default:
      break;
  }

  // clamp distance
  if (builder->vx == 1) {
    distance = min(distance, builder->limit - builder->x);
  } else if (builder->vx == -1) {
    distance = min(distance, builder->x);
  } else if (builder->vy == 1) {
    distance = min(distance, builder->limit - builder->y);
  } else if (builder->vy == -1) {
    distance = min(distance, builder->y);
  }

  // eusures moving an even number of steps
  if (distance % 2) {
    distance--;
  }

  assert(distance >= 2);

  builder->remain = distance;
}

static int calcWeight(MazeBuilder* builder, char axis) {
  int weight = 0;

  for (size_t i = 0; i < builder->spawnerCount; i++) {
    // skip self
    if (i == builder->spawnerIndex) {
      continue;
    }

    size_t spawnerPos, builderPos;

    if (axis == 'x') {
      spawnerPos = builder->spawners[i].x;
      builderPos = builder->x;
    } else {
      spawnerPos = builder->spawners[i].y;
      builderPos = builder->y;
    }

    weight += sign(spawnerPos - builderPos);
  }

  return weight;
}

static Direction chooseDirection(MazeBuilder* builder, Direction bidir) {
  switch (bidir) {
    case DIR_HORIZONTAL:
      return calcWeight(builder, 'x') > 0 ? DIR_RIGHT : DIR_LEFT;

    case DIR_VERTICAL:
      return calcWeight(builder, 'y') > 0 ? DIR_DOWN : DIR_UP;

    default:
      return DIR_NONE;
  }
}

static void turnBuilder(MazeBuilder* builder) {
  Direction current = builder->direction;
  Direction candidates = (current & DIR_VERTICAL) ? DIR_HORIZONTAL : DIR_VERTICAL;
  Direction next = filterDirections(builder->x, builder->y, builder->limit, candidates);

  builder->direction = isSingleDirection(next) ? next : chooseDirection(builder, next);

  updateDistance(builder);
}

static void moveBuilder(MazeBuilder* builder) {
  if (!builder->remain) {
    turnBuilder(builder);
  }

  builder->x += builder->vx;
  builder->y += builder->vy;
  builder->remain--;
}

// ============ Builder End ============ //

static void initTiles(Tile tiles[MAP_SIZE][MAP_SIZE]) {
  for (size_t i = 0; i < MAP_SIZE; i++) {
    for (size_t j = 0; j < MAP_SIZE; j++) {
      tiles[i][j] = TILE_WALL;
    }
  }
}

/**
 * Splits map into 2 rows * n blocks and pick a random tile from each block
 *
 *                    +---+---+---+-...-+----+
 *  first row:  index=| 0 | 2 | 4 | --> | 2n |
 *                    +---+---+---+-...-+----+
 *
 *                    +------+-...-+---+---+---+
 *  second row: index=| 2n+1 | <-- | 5 | 3 | 1 |
 *                    +------+-...-+---+---+---+
 */
static void initSpawners(size_t count,
                         Spawner spawners[],
                         Tile tiles[MAP_SIZE][MAP_SIZE]) {
  // split each row into at least 2 blocks
  const size_t splits = (size_t)max(2, round(count / 2.0));

  // shrink to [0.1, 0.9] area of the block to avoid spawners being too close
  const double lower = 0.1;
  const double upper = 0.9;

  const double halfBlockWidth = (MAP_SIZE - 1.0) / splits / 2.0;
  const double halfBlockHeight = (MAP_SIZE - 1.0) / 4.0;

  for (size_t i = 0; i < count; i++) {
    // x
    size_t blockCol = i % 2 == 0 ? (i / 2) : (splits - 1 - i / 2);
    size_t halfX = randomInt(
      ceil(halfBlockWidth * (blockCol + lower)),
      floor(halfBlockWidth * (blockCol + upper))
    );

    // y
    size_t blockRow = i % 2;
    size_t halfY = randomInt(
      ceil(halfBlockHeight * (blockRow + lower)),
      floor(halfBlockHeight * (blockRow + upper))
    );

    // map into even-numbered coordinates
    size_t x = halfX * 2;
    size_t y = halfY * 2;

    Spawner s = { x, y };

    spawners[i] = s;
    tiles[y][x] = TILE_KERNEL;
  }
}

static void initBuilders(size_t minDistance,
                         size_t maxDistance,
                         size_t spawnerCount,
                         Spawner spawners[],
                         BuilderList* builders) {
  size_t limit = MAP_SIZE - 1;

  for (size_t i = 0; i < spawnerCount; i++) {
    Spawner s = spawners[i];

    Direction directions[4];
    unsigned int dirCount = extractDirections(filterDirections(s.x, s.y, limit, DIR_ALL), directions);

    shuffle(directions, dirCount, sizeof(Direction));

    unsigned int dirSelected = randomInt(2, (int)dirCount);

    for (size_t j = 0; j < dirSelected; j++) {
      MazeBuilder* builder = malloc(sizeof(MazeBuilder));
      builder->x = s.x;
      builder->y = s.y;
      builder->limit = limit;
      builder->minDistance = minDistance;
      builder->maxDistance = maxDistance;
      builder->direction = directions[j];
      builder->spawners = spawners;
      builder->spawnerIndex = i;
      builder->spawnerCount = spawnerCount;
      builder->vx = builder->vy = builder->remain = 0;

      BuilderNode* node = (BuilderNode*)createNode();
      node->data = builder;

      listAppend((List*)builders, (Node*)node);

      updateDistance(builder);
    }
  }
}

static void generateMap(Tile tiles[MAP_SIZE][MAP_SIZE], BuilderList* builders) {
  while (builders->count) {
    BuilderNode* node = builders->head;

    while (node) {
      // record `next` in case that node is deleted
      BuilderNode* next = node->next;
      MazeBuilder* b = node->data;

      moveBuilder(b);

      if (tiles[b->y][b->x] & TILE_OPEN) {
        // remove collided builders
        listDelete((List*)builders, (Node*)node);
      } else {
        tiles[b->y][b->x] = TILE_BEAN;
      }

      node = next;
    }
  }
}

static size_t fixMap(Tile tiles[MAP_SIZE][MAP_SIZE], Spawner* startPoint) {
  FloodState* state = floodFill(tiles, startPoint->x, startPoint->y);

  for (size_t i = 0; i < MAP_SIZE; i++) {
    for (size_t j = 0; j < MAP_SIZE; j++) {
      if (tiles[i][j] & TILE_OPEN && !state->visited[i][j]) {
        tiles[i][j] = TILE_WALL;
      }
    }
  }

  size_t pathLength = state->pathLength;

  floodDestory(state);

  return pathLength;
}

size_t initMaze(size_t spawnerCount,
                size_t minDistance,
                size_t maxDistance,
                Tile tiles[MAP_SIZE][MAP_SIZE]) {
  Spawner* spawners = malloc(sizeof(Spawner) * spawnerCount);
  BuilderList* builders = (BuilderList*)createList();

  initTiles(tiles);
  initSpawners(spawnerCount, spawners, tiles);
  initBuilders(minDistance, maxDistance, spawnerCount, spawners, builders);

  generateMap(tiles, builders);

  size_t pathLength = fixMap(tiles, &spawners[0]);

  free(spawners);
  listDestory((List*)builders);

  return pathLength;
}
