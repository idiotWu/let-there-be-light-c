/**
 * @file
 * @brief 迷路の生成など
 *
 * @see <http://www.contralogic.com/2d-pac-man-style-maze-generation/>
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "config.h"

#include "maze/maze.h"

#include "maze/tile.h"
#include "maze/direction.h"
#include "maze/floodfill.h"
#include "util/list.h"
#include "util/util.h"

/**
 * @internal
 * @brief 迷路のビルダー
 */
typedef struct MazeBuilder {
  //! ビルダーの x 座標
  int x;
  //! ビルダーの y 座標
  int y;

  //! x 方向における移動スピード
  int vx;
  //! y 方向における移動スピード
  int vy;

  //! 迷路の大きさ制限
  int limit;
  //! 残りの歩数
  int remain;

  //! 最短走行距離
  int minDistance;
  //! 最大走行距離
  int maxDistance;

  //! ビルダーの向き
  Direction direction;
  //! 生成器の配列
  vec2i* spawners;
  //! このビルダーを作った生成器のインデックス
  int spawnerIndex;
  //! 生成器の数
  int spawnerCount;
} MazeBuilder;

/**
 * @brief 複数の方向から進まない方向を取り除く
 *
 * @param x           現在のビルダーの x 座標
 * @param y           現在のビルダーの y 座標
 * @param limit       迷路の大きさ制限
 * @param candidates  選択できる方向（複数）
 *
 * @return Direction  行ける方向
 */
static Direction filterDirections(int x, int y, int limit, Direction candidates) {
  if (x == 0 || x == 1 /* odd numbers of distance are not allowed */) {
    clearBits(candidates, DIR_LEFT);
  } else if (x == limit || x == limit - 1) {
    clearBits(candidates, DIR_RIGHT);
  }

  if (y == 0 || y == 1) {
    clearBits(candidates, DIR_DOWN);
  } else if (y == limit || y == limit - 1) {
    clearBits(candidates, DIR_UP);
  }

  return candidates;
}

// ============ Builder Begin ============ //

/**
 * @brief ビルダーの次回の走行距離を計算する
 *
 * @param builder 目標のビルダー
 */
static void updateDistance(MazeBuilder* builder) {
  int distance = randomInt(builder->minDistance, builder->maxDistance);

  assert(distance != 0);

  vec2i velocity = directionToDelta(builder->direction);

  builder->vx = velocity.x;
  builder->vy = velocity.y;

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

/**
 * @brief ビルダーの位置において，横軸（方向）または縦軸（方向）のウェイトを計算する
 *
 * @details この関数の戻り値はどの方向において生成器の数が多いかを表している．
 * - x 軸に対して，左側に生成器の数が多い場合は負の値を，右側が多い場合は正の値を，両側が等しい場合は 0 を返す．
 * - y 軸に対して，下側に生成器の数が多い場合は負の値を，上側が多い場合は正の値を，両側が等しい場合は 0 を返す．
 *
 * @param builder 目標のビルダー
 * @param axis    横軸 `'x'`，または縦軸 `'y'`
 *
 * @return int ウェイト
 */
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

/**
 * @brief ビルダーが曲がるときに，他の生成器に近づける方向（ウェイトの大きい）を選ぶ
 *
 * @param builder 目標のビルダー
 * @param bidir   選択できる両方向
 *
 * @return Direction 計算で得た片方向
 */
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

/**
 * @brief ビルダーを 90 度回転させる
 *
 * @param builder 目標のビルダー
 */
static void turnBuilder(MazeBuilder* builder) {
  Direction current = builder->direction;
  Direction candidates = (current & DIR_VERTICAL) ? DIR_HORIZONTAL : DIR_VERTICAL;
  Direction next = filterDirections(builder->x, builder->y, builder->limit, candidates);

  builder->direction = isSingleDirection(next) ? next : chooseDirection(builder, next);

  updateDistance(builder);
}

/**
 * @brief ビルダーを一回進める
 *
 * @param builder 目標のビルダー
 */
static void moveBuilder(MazeBuilder* builder) {
  if (!builder->remain) {
    turnBuilder(builder);
  }

  builder->x += builder->vx;
  builder->y += builder->vy;
  builder->remain--;
}

// ============ Builder End ============ //

/**
 * @brief 生成器を初期化する
 *
 * @details 迷路を可能な限り広くするため，生成器同士の距離を大きくする必要がある．
 * ここでは，生成器の数を 2n とし，迷路を 2 列に分け，各列に n 個のブロックを分割する．
 * そして，下記のルールに従って生成器を置く．
 * ```
 *                       +---+---+---+-...-+----+
 *  1 行目: 生成器の index=| 0 | 2 | 4 | --> | 2n |
 *                       +---+---+---+-...-+----+
 *
 *                       +------+-...-+---+---+---+
 *  2 行目: 生成器の index=| 2n+1 | <-- | 5 | 3 | 1 |
 *                       +------+-...-+---+---+---+
 * ```
 *
 * @param count    生成器の数
 * @param spawners 生成器の配列
 * @param tiles    迷路の構成
 */
static void initSpawners(int count, vec2i spawners[], Tile tiles[MAZE_SIZE][MAZE_SIZE]) {
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
    tiles[s.y][s.x] = TILE_KERNEL | TILE_PATH;
  }
}

/**
 * @brief ビルダーを初期化する
 *
 * @param minDistance  最短走行距離
 * @param maxDistance  最大走行距離
 * @param spawnerCount 生成器の数
 * @param spawners     生成器の配列
 * @param builders     ビルダーのリスト
 */
static void initBuilders(int minDistance, int maxDistance,
                         int spawnerCount, vec2i spawners[],
                         List* builders) {
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

      Node* node = createNode();
      node->data = builder;

      listAppend(builders, node);

      updateDistance(builder);
    }
  }
}

/**
 * @brief 迷路を生成する
 *
 * @param tiles    タイルの保存先
 * @param builders ビルダーのリスト
 */
static void generateMap(Tile tiles[MAZE_SIZE][MAZE_SIZE], List* builders) {
  while (builders->count) {
    ListIterator it = createListIterator(builders);

    while (!it.done) {
      Node* node = it.next(&it);
      MazeBuilder* b = node->data;

      moveBuilder(b);

      if (tiles[b->y][b->x] & TILE_OPEN) {
        // remove collided builders
        listDelete(builders, node);
      } else {
        tiles[b->y][b->x] = TILE_COIN | TILE_PATH;
      }
    }
  }
}

/**
 * @brief 孤立した部分（離島）を迷路から排除する
 *
 * @param tiles       迷路の構成
 * @param startPoint  チェックの出発点
 *
 * @return int 開いているタイルの数
 */
static int fixMap(Tile tiles[MAZE_SIZE][MAZE_SIZE], vec2i* startPoint) {
  FloodState* state = floodFill(tiles, startPoint->x, startPoint->y);

  for (size_t i = 0; i < MAZE_SIZE; i++) {
    for (size_t j = 0; j < MAZE_SIZE; j++) {
      if (tiles[i][j] & TILE_OPEN &&
          state->depthMap[i][j] == FLOOD_DEPTH_UNVISITED) {
        tiles[i][j] = TILE_WALL;
      }
    }
  }

  int pathLength = state->pathLength;

  floodDestory(state);

  return pathLength;
}

int initMaze(int spawnerCount,
             int minDistance, int maxDistance,
             Tile tiles[MAZE_SIZE][MAZE_SIZE]) {
  vec2i* spawners = malloc(sizeof(vec2i) * spawnerCount);
  List* builders = createList();

  memset(tiles, TILE_WALL, sizeof(tiles[0][0]) * MAZE_SIZE * MAZE_SIZE);

  initSpawners(spawnerCount, spawners, tiles);
  initBuilders(minDistance, maxDistance, spawnerCount, spawners, builders);

  generateMap(tiles, builders);

  int pathLength = fixMap(tiles, &spawners[0]);

  free(spawners);
  listDestory(builders);

  return pathLength;
}

// check if the given position is path
bool isPath(Tile tiles[MAZE_SIZE][MAZE_SIZE],
            int x, int y) {
  if (x < 0 || x >= MAZE_SIZE ||
      y < 0 || y >= MAZE_SIZE) {
    return false;
  }

  return tiles[y][x] & TILE_OPEN;
}

Direction getAvailableDirection(Tile tiles[MAZE_SIZE][MAZE_SIZE],
                                int x, int y) {
  Direction direction = DIR_NONE;

  if (isPath(tiles, x, y - 1)) {
    direction = DIR_DOWN;
  } else if (isPath(tiles, x, y + 1)) {
    direction = DIR_UP;
  } else if (isPath(tiles, x - 1, y)) {
    direction = DIR_LEFT;
  } else if (isPath(tiles, x + 1, y)) {
    direction = DIR_RIGHT;
  }

  return direction;
}
