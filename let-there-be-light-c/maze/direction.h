/**
 * @file
 * @brief 方向の定義（ヘッダファイル）
 */
#ifndef DIRECTION_H
#define DIRECTION_H

#include <stdbool.h>
#include <stddef.h>

#include "util/util.h"

/**
 * @brief 方向
 */
typedef enum Direction {
  //! 無方向
  DIR_NONE  = 0,      /* 0000 */
  //! 上
  DIR_UP    = 1 << 0, /* 0001 */
  //! 右
  DIR_RIGHT = 1 << 1, /* 0010 */
  //! 下
  DIR_DOWN  = 1 << 2, /* 0100 */
  //! 左
  DIR_LEFT  = 1 << 3, /* 1000 */

  //! 縦方向
  DIR_VERTICAL   = DIR_UP | DIR_DOWN,
  //! 横方向
  DIR_HORIZONTAL = DIR_LEFT | DIR_RIGHT,
  //! すべての方向
  DIR_ALL        = DIR_UP | DIR_RIGHT | DIR_DOWN | DIR_LEFT
} Direction;

/**
 * @brief 片方向であるを判断する
 *
 * @param dir 判断したい方向
 *
 * @return true  片方向である
 * @return false 片方向ではない
 */
bool isSingleDirection(Direction dir);

/**
 * @brief 複数の方向の組み合わせを展開する
 *
 * @param dir 方向の組み合わせ
 * @param res 結果の保存先（最大4つ）
 *
 * @return int 展開後の方向の数
 */
int extractDirections(Direction dir, Direction res[4]);

/**
 * @brief 方向をデルタ（変位）に変換する
 *
 * @param dir 変換したい方向
 *
 * @return vec2i 変換後のデルタ
 */
vec2i directionToDelta(Direction dir);

/**
 * @brief デルタを方向に変換する
 *
 * @param delta 変換したいデルタ
 *
 * @return Direction 変換後の方向
 */
Direction deltaToDirection(vec2i delta);

#endif
