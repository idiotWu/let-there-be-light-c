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

typedef struct MazeBuilder {
  int x;
  int y;

  int vx;
  int vy;

  int limit;
  int remain;

  int minDistance;
  int maxDistance;

  Direction direction;
  vec2i* spawners;
  int spawnerIndex;
  int spawnerCount;
} MazeBuilder;

defNode(BuilderNode, MazeBuilder);
defList(BuilderList, BuilderNode);

static Direction filterDirections(int x,
                                  int y,
                                  int limit,
                                  Direction candidates) {
  if (x == 0 || x == 1 /* odd numbers of distance are not allowed */) {
    candidates &= ~DIR_LEFT; // clear the specific bit
  } else if (x == limit || x == limit - 1) {
    candidates &= ~DIR_RIGHT;
  }

  if (y == 0 || y == 1) {
    candidates &= ~DIR_DOWN;
  } else if (y == limit || y == limit - 1) {
    candidates &= ~DIR_UP;
  }

  return candidates;
}

// ============ Builder Begin ============ //

static void updateDistance(MazeBuilder* builder) {
  int distance = randomInt(builder->minDistance, builder->maxDistance);

  assert(distance != 0);

  switch (builder->direction) {
    case DIR_UP:
      builder->vx = 0;
      builder->vy = 1;
      break;

    case DIR_DOWN:
      builder->vx = 0;
      builder->vy = -1;
      break;

    case DIR_RIGHT:
      builder->vx = 1;
      builder->vy = 0;
      break;

    case DIR_LEFT:
      builder->vx = -1;
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

  for (int i = 0; i < builder->spawnerCount; i++) {
    // skip self
    if (i == builder->spawnerIndex) {
      continue;
    }

    int spawnerPos, builderPos;

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
      return calcWeight(builder, 'y') > 0 ? DIR_UP : DIR_DOWN;

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

static void initTiles(Tile tiles[MAZE_SIZE][MAZE_SIZE]) {
  for (size_t i = 0; i < MAZE_SIZE; i++) {
    for (size_t j = 0; j < MAZE_SIZE; j++) {
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
static void initSpawners(int count,
                         vec2i spawners[],
                         Tile tiles[MAZE_SIZE][MAZE_SIZE]) {
  // max odd number of index
  const int maxIndex = floor(MAZE_SIZE / 2) * 2 - 1;

  // split each row into at least 2 blocks
  const int splits = max(2, round(count / 2.0));

  // shrink to [0.1, 0.9] area of the block to avoid spawners being too close
  const double lower = 0.1;
  const double upper = 0.9;

  const double halfBlockWidth = (MAZE_SIZE - 3.0) / splits / 2.0;
  const double halfBlockHeight = (MAZE_SIZE - 3.0) / 4.0;

  for (int i = 0; i < count; i++) {
    // x
    int blockCol = i % 2 == 0 ? (i / 2) : (splits - 1 - i / 2);
    int halfX = randomInt(
      ceil(halfBlockWidth * (blockCol + lower)),
      floor(halfBlockWidth * (blockCol + upper))
    );

    // y
    int blockRow = i % 2;
    int halfY = randomInt(
      ceil(halfBlockHeight * (blockRow + lower)),
      floor(halfBlockHeight * (blockRow + upper))
    );

    // map into odd-numbered coordinates
    vec2i s;

    s.x = clamp(halfX * 2 + 1, 1, maxIndex);
    s.y = clamp(halfY * 2 + 1, 1, maxIndex);

    spawners[i] = s;
    tiles[s.y][s.x] = TILE_KERNEL;
  }
}

static void initBuilders(int minDistance,
                         int maxDistance,
                         int spawnerCount,
                         vec2i spawners[],
                         BuilderList* builders) {
  int limit = MAZE_SIZE - 1;

  for (int i = 0; i < spawnerCount; i++) {
    vec2i s = spawners[i];

    Direction directions[4];
    int dirCount = extractDirections(filterDirections(s.x, s.y, limit, DIR_ALL), directions);

    shuffle(directions, dirCount, sizeof(Direction));

    int dirSelected = randomInt(2, dirCount);

    for (int j = 0; j < dirSelected; j++) {
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

      BuilderNode* node = createNode();
      node->data = builder;

      listAppend(builders, node);

      updateDistance(builder);
    }
  }
}

static void generateMap(Tile tiles[MAZE_SIZE][MAZE_SIZE], BuilderList* builders) {
  while (builders->count) {
    ListIterator it = createListIterator(builders);

    while (!it.done) {
      BuilderNode* node = it.next(&it);
      MazeBuilder* b = node->data;

      moveBuilder(b);

      if (tiles[b->y][b->x] & TILE_OPEN) {
        // remove collided builders
        listDelete(builders, node);
      } else {
        tiles[b->y][b->x] = TILE_COIN;
      }
    }
  }
}

static int fixMap(Tile tiles[MAZE_SIZE][MAZE_SIZE], vec2i* startPoint) {
  FloodState* state = floodFill(tiles, startPoint->x, startPoint->y);

  for (size_t i = 0; i < MAZE_SIZE; i++) {
    for (size_t j = 0; j < MAZE_SIZE; j++) {
      if (tiles[i][j] & TILE_OPEN && !state->visited[i][j]) {
        tiles[i][j] = TILE_WALL;
      }
    }
  }

  int pathLength = state->pathLength;

  floodDestory(state);

  return pathLength;
}

int initMaze(int spawnerCount,
             int minDistance,
             int maxDistance,
             Tile tiles[MAZE_SIZE][MAZE_SIZE]) {
  vec2i* spawners = malloc(sizeof(vec2i) * spawnerCount);
  BuilderList* builders = createList();

  initTiles(tiles);
  initSpawners(spawnerCount, spawners, tiles);
  initBuilders(minDistance, maxDistance, spawnerCount, spawners, builders);

  generateMap(tiles, builders);

  int pathLength = fixMap(tiles, &spawners[0]);

  free(spawners);
  listDestory(builders);

  return pathLength;
}
