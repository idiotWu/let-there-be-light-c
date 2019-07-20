/**
 * @file
 * @brief Flood-fill アルゴリズム（ヘッダファイル）
 */
#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <stdbool.h>
#include <stddef.h>

#include "config.h"

#include "maze/tile.h"
#include "util/list.h"

//! 未訪問のタイル
#define FLOOD_DEPTH_UNVISITED  -1

/**
 * @brief Flood の状態を表す
 */
typedef struct FloodState {
  //! Flood の先頭
  List* frontiers;
  //! 迷路の構成
  const Tile (*tiles)[MAZE_SIZE];
  //! 各タイルに対して水が流れてくるまでの歩数
  int depthMap[MAZE_SIZE][MAZE_SIZE];
  //! 開いているタイルの数
  int pathLength;
  //! @private 現在の歩数
  int currentDepth;
  //! Flood が全部のタイルに流れ込んだか
  bool finished;
} FloodState;

/**
 * @brief Flood を一回進める
 *
 * @param state Flood の状態
 */
void floodForward(FloodState* state);


/**
 * @brief Flood-fill を指定のタイルで生成する
 *
 * @param tiles 迷路の構成
 * @param x     タイルの x 座標
 * @param y     タイルの y 座標
 *
 * @return FloodState* Flood の状態
 */
FloodState* floodGenerate(const Tile tiles[MAZE_SIZE][MAZE_SIZE], int x, int y);

/**
 * @brief Flood-fill を指定のタイルから自動的に実行する
 *
 * @param tiles 迷路の構成
 * @param x     タイルの x 座標
 * @param y     タイルの y 座標
 *
 * @return FloodState* Flood の状態
 */
FloodState* floodFill(Tile tiles[MAZE_SIZE][MAZE_SIZE], int x, int y);

/**
 * @brief 指定の Flood-fill を廃棄する
 *
 * @param state 廃棄したい Flood
 */
void floodDestory(FloodState* state);

#endif
