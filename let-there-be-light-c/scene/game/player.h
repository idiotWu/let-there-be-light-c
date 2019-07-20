/**
 * @file
 * @brief プレイヤーの定義 ＜ヘッダファイル＞
 */
#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#include "maze/direction.h"

/**
 * @brief プレイヤーを表すのオブジェクト
 */
typedef struct Player {
  //! プレイヤーの x 座標
  double x;
  //! プレイヤーの y 座標
  double y;
  //! プレイヤーが静止している
  bool idle;
  //! プレイヤーが凍った地面を走っている
  bool spoiled;
  //! プレイヤーのスプライトの状態（フレーム）
  int spriteState;
  //! プレイヤーの向き
  Direction direction;
} Player;

/**
 * @brief プレイヤーを移動させる
 *
 * @param deltaX 変位の x 分量
 * @param deltaY 変位の y 分量
 */
void movePlayer(int deltaX, int deltaY);

/**
 * @brief 各タイルにおけるプレイヤーまでの最小歩数を更新する
 *
 * @details この関数は \ref State.stepsFromPlayer "GameState.stepsFromPlayer" を更新する
 */
void updateStepsFromPlayer(void);

/**
 * @brief プレイヤーを初期化する
 */
void initPlayer(void);

/**
 * @brief プレイヤーを削除する
 */
void destroyPlayer(void);

#endif
