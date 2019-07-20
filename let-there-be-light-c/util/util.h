/**
 * @file
 * @brief ユーティリティ ＜ヘッダファイル＞
 */
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define UNUSED(x) (void)(x)

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

/**
 * @brief 目標の変数に特定のビットをセットする
 *
 * @param target 目標の変数
 * @param mask   セットするビット
 */
#define setBits(target, mask) (target |= mask)

/**
 * @brief 目標の変数に特定のビットをクリアする
 *
 * @param target 目標の変数
 * @param mask   クリアするビット
 */
#define clearBits(target, mask) (target &= ~mask)

/**
 * @brief int 型の2次元ベクトル
 */
typedef struct vec2i {
  //! x 座標
  int x;
  //! y 座標
  int y;
} vec2i;

/**
 * @brief double 型の2次元ベクトル
 */
typedef struct vec2d {
  //! x 座標
  double x;
  //! y 座標
  double y;
} vec2d;

/**
 * @brief 指定される区間 `[min, max]` から double 型の乱数を生成する
 *
 * @param min 下限
 * @param max 上限
 *
 * @return double 乱数
 */
double randomBetween(double min, double max);
/**
 * @brief 指定される区間 `[lower, upper]` から int 型の乱数を生成する
 *
 * @param lower 下限
 * @param upper 上限
 *
 * @return int 乱数
 */
int randomInt(int lower, int upper);

/**
 * @brief 2つの変数の値を交換する
 *
 * @param a  1つ目の変数
 * @param b  2つ目の変数
 * @param bytes  変数のサイズ
 */
void swap(void* a, void* b, size_t bytes);

/**
 * @brief 配列の要素をランダムに並べ替える
 *
 * @param array           目標の配列
 * @param length          配列の長さ
 * @param bytesPerElement 各要素のサイズ
 */
void shuffle(void* array, size_t length, size_t bytesPerElement);

/**
 * @brief 数値が正，負，0 のどれであるかを示す符号を求める
 *
 * @details 使用例：
 * ```c
 * sign(0);      // 0
 * sign(1024);   // 1
 * sign(-65536); // -1
 * ```
 *
 * @param num 数値
 *
 * @return int 符号
 */
int sign(double num);

/**
 * @brief [min, max] の区間に制限された値を返す
 *
 * @details 使用例：
 * ```c
 * clamp(0.5, 0.0, 1.0);  // 0.5
 * clamp(1.5, 0.0, 1.0);  // 1.0
 * clamp(-0.5, 0.0, 1.0); // 0.0
 * ```
 *
 * @param val 元の値
 * @param min 下限
 * @param max 上限
 *
 * @return double 制限された値
 */
double clamp(double val, double min, double max);

/**
 * @brief 2つの点 a, b の距離を計算する
 *
 * @param x1 点 a の x 座標
 * @param y1 点 a の y 座標
 * @param x2 点 b の x 座標
 * @param y2 点 b の y 座標
 *
 * @return double a，b の距離
 */
double distance(double x1, double y1, double x2, double y2);

/**
 * @brief 何もしない関数
 *
 * @details ᕕ( ᐛ )ᕗ
 */
void noop(void);

#endif
