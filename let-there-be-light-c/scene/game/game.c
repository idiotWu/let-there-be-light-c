/**
 * @file
 * @brief ゲームのコントローラー
 */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "glut.h"

#include "config.h"
#include "state.h"

#include "scene/game/game.h"

#include "scene/game/enemy.h"
#include "scene/game/player.h"
#include "scene/game/render.h"
#include "scene/game/keyboard.h"
#include "scene/game/difficulty.h"
#include "scene/game-over/game-over.h"
#include "scene/scene.h"
#include "render/transition.h"
#include "util/util.h"
#include "maze/maze.h"
#include "maze/tile.h"
#include "maze/floodfill.h"
#include "maze/direction.h"
#include "render/engine.h"
#include "render/texture.h"
#include "render/fx.h"

/**
 * @brief ゲームを初期化する
 */
static void initGame(void);
/**
 * @brief ゲームを更新する
 */
static void updateGame(void);
/**
 * @brief ゲームシーンから離れる時の処理
 */
static void leaveGame(void);

//! ゲームシーン
static Scene game = {
  .init = initGame,
  .update = updateGame,
  .render = renderGame,
  .destroy = leaveGame,
};

Scene* gameScene = &game;

// ============ Maze Initialization ============ //

/**
 * @brief アイテムを迷路の中に配置する
 */
static void initItems(void) {
  Tile (*tiles)[MAZE_SIZE] = GameState.maze;

  double itemDesity = getItemDensity();

  int openCount = 0;
  GameState.remainItem = 0;

  for (int y = 0; y < MAZE_SIZE; y++) {
    for (int x = 0; x < MAZE_SIZE; x++) {
      if (tiles[y][x] & TILE_OPEN) {
        vec2i pos = { x, y };
        GameState.openTiles[openCount++] = pos;

        // clear items randomly
        if (randomBetween(0, 1) >= itemDesity) {
          clearBits(tiles[y][x], TILE_ITEM);
        } else {
          GameState.remainItem++;
        }
      } else if (y != MAZE_SIZE - 1 && tiles[y + 1][x] & TILE_OPEN) {
        tiles[y][x] = TILE_SHADOW;
      }
    }
  }
}

// ============ Visible Radius ============ //

/**
 * @brief 見える範囲を拡大するアニメーションを更新する（補間のアニメーション）
 *
 * @details アニメーションの進行状態により見える範囲を少しずつ拡大する
 *
 * @param animation アニメーション
 */
static void expandVisionUpdate(Animation* animation) {
  double* fromRadius = animation->from;
  double* deltaRadius = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.visibleRadius = *fromRadius + *deltaRadius * percent;
}

/**
 * @brief 見える範囲を拡大する
 *
 * @param radius   拡大後の半径（エネルギー残量）
 * @param duration アニメーションの持続時間
 */
static void expandVision(double radius, double duration) {
  double* fromRadius = malloc(sizeof(double));
  double* deltaRadius = malloc(sizeof(double));

  *fromRadius = GameState.visibleRadius;
  *deltaRadius = radius - GameState.visibleRadius;

  Animation* animation = createAnimation60FPS(duration, 1);

  animation->from = fromRadius;
  animation->delta = deltaRadius;
  animation->render = expandVisionUpdate;
}

// ============ Keyboard Handler ============ //

/**
 * @brief 押している方向キーを読み取り，プレイヤーを移動させる
 */
static void readKeyboard(void) {
  Direction moveDir = getPressedKeys();

  if (moveDir == DIR_NONE) {
    return;
  }

  int x = GameState.player.x;
  int y = GameState.player.y;

  vec2i delta = { 0, 0 };

  if (moveDir & DIR_LEFT) {
    delta.x--;
  }
  if (moveDir & DIR_RIGHT) {
    delta.x++;
  }
  if (moveDir & DIR_UP) {
    delta.y++;
  }
  if (moveDir & DIR_DOWN) {
    delta.y--;
  }

  if (!isPath(GameState.maze, x + delta.x, y)) {
    delta.x = 0;
  }
  if (!isPath(GameState.maze, x, y + delta.y)) {
    delta.y = 0;
  }

  if (delta.x || delta.y) {
    GameState.player.idle = false;
    movePlayer(delta.x, delta.y);
  }
}

/**
 * @brief アイテムの状態を更新する
 */
static void updateItems(void) {
  int x = GameState.player.x;
  int y = GameState.player.y;

  Tile tile = GameState.maze[y][x];

  if (tile & TILE_ITEM) {
    // pick up item
    if (tile & TILE_KERNEL) {
      fxFlood(x, y);
      expandVision(min(GameState.visibleRadius + 1.0, MAX_VISIBLE_RADIUS), 500);
    }

    clearBits(GameState.maze[y][x], TILE_ITEM);
    GameState.remainItem--;
  }
}

// ============ Misc ============ //

static void initGame(void) {
  int spawnerCount = getSpawnerCount();
  int minDistance = getMinBuilderDistance();
  int maxDistance = minDistance * 2;

  int pathLength;
  int tried = 0;

  do {
    pathLength = initMaze(spawnerCount, minDistance, maxDistance, GameState.maze);
    tried++;
  } while(pathLength < MIN_PATH_LENGTH && tried < 5);

  free(GameState.openTiles);

  GameState.paused = false;
  GameState.pathLength = pathLength;
  GameState.openTiles = malloc(pathLength * sizeof(*GameState.openTiles));
  GameState.visibleRadius = getInitialVisibleRadius();

  initItems();
  initPlayer();
  initEnemy();

  bindGameKeyboardHandlers();
}

/**
 * @brief ゲームクリアの処理
 *
 * @details この関数は遅れて実行される
 *
 * @param _ `delay` 関数のためのプレースホルダー（使われていない）
 */
static void gameClear(void* _) {
  UNUSED(_);

  if (GameState.level.minor < LEVEL_INTERVAL) {
    GameState.level.minor++;
  } else {
    GameState.level.major++;
    GameState.level.minor = 1;
  }

  GameState.paused = true;

  levelTransition();
}

static void updateGame(void) {
  if (GameState.paused) {
    return;
  }

  GameState.visibleRadius -= getVisibleRadiusRuducingRate();

  // game over
  if (GameState.visibleRadius <= 0) {
    return switchScene(gameOverScene);
  }

  Player* player = &GameState.player;

  player->spoiled = (GameState.maze[(int)player->y][(int)player->x] & TILE_SPOILED);

  // move player when it's idle(not moving)
  if (player->idle) {
    updateItems();
    readKeyboard();
  }

  // activate sleeping enemies
  activateEnemies();

  // stage clear
  if (GameState.remainItem == 0) {
    player->spoiled = false;
    GameState.paused = true;
    GameState.lastVisibleRadius = GameState.visibleRadius;

    // show maze
    expandVision(MAZE_SIZE * 10, 3000);
    // remove all enemies
    destroyEnemy(true);
    // clear spoiled tiles
    fxFlood(player->x, player->y);

    delay(5000, gameClear, NULL);
  }
}

static void leaveGame(void) {
  destroyPlayer();
  destroyEnemy(false);
  removeGameKeyboardHandlers();
}
