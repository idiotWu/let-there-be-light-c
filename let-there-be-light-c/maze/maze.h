/**
 * @file
 * @brief 迷路の生成など ＜ヘッダファイル＞
 *
 * @see <http://www.contralogic.com/2d-pac-man-style-maze-generation/>
 */
#ifndef MAZE_H
#define MAZE_H

#include <stddef.h>
#include <stdbool.h>

#include "config.h"

#include "maze/tile.h"
#include "maze/direction.h"

/**
 * @brief 迷路を生成する
 *
 * @param spawnerCount 生成器の数
 * @param minDistance  ビルダーの最短走行距離
 * @param maxDistance  ビルダーの最大走行距離
 * @param tiles        迷路の構成の保存先
 *
 * @return int 空いているタイルの数
 */
int initMaze(int spawnerCount,
             int minDistance, int maxDistance,
             Tile tiles[MAZE_SIZE][MAZE_SIZE]);

/**
 * @brief 指定のタイルが空いているのかをチェックする
 *
 * @param tiles 迷路の構成
 * @param x     タイルの x 座標
 * @param y     タイルの y 座標
 *
 * @return true  タイルが空いている（道である）
 * @return false タイルが閉じている（壁である）
 */
bool isPath(Tile tiles[MAZE_SIZE][MAZE_SIZE], int x, int y);

/**
 * @brief 指定のタイルにおいて，移動できる方向をチェックする
 *
 * @param tiles 迷路の構成
 * @param x     タイルの x 座標
 * @param y     タイルの y 座標
 *
 * @return Direction 移動できる方向
 */
Direction getAvailableDirection(Tile tiles[MAZE_SIZE][MAZE_SIZE], int x, int y);

#endif
