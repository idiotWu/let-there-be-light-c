/**
 * @file
 * @brief 敵の定義 ＜ヘッダファイル＞
 */
#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>

#include "util/list.h"
#include "render/engine.h"
#include "maze/direction.h"

/**
 * @brief 敵を表すのオブジェクト
 */
typedef struct Enemy {
  //! 敵の x 座標
  double x;
  //! 敵の y 座標
  double y;
  //! 敵の向き
  Direction direction;

  //! 敵が蘇った
  bool activated;

  //! 敵のスプライトの状態（フレーム）
  int spriteState;
  //! 残りの移動歩数
  int remainSteps;

  //! 移動のアニメーション
  Animation* movingAnimation;
} Enemy;

/**
 * @brief 敵を起こす
 */
void activateEnemies(void);

/**
 * @brief 敵の生成器を初期化する
 */
void initEnemy(void);

/**
 * @brief 敵の生成器を廃棄する
 *
 * @param shouldExplode 残る敵を爆発させる
 */
void destroyEnemy(bool shouldExplode);

#endif
