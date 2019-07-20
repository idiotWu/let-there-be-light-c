/**
 * @file
 * @brief ゲームの難易度の計算 ＜ヘッダファイル＞
 */
#ifndef DIFFICULTY_H
#define DIFFICULTY_H

/**
 * @brief 迷路の生成器の数を計算する
 *
 * @return int 生成器の数
 */
int getSpawnerCount(void);

/**
 * @brief 迷路のビルダーの最短走行距離を計算する
 *
 * @return int 最短走行距離
 */
int getMinBuilderDistance(void);

/**
 * @brief ステージが始まる時のエネルギー残量を計算する
 *
 * @return double エネルギー残量
 */
double getInitialVisibleRadius(void);

/**
 * @brief エネルギー残量の減らすスピードを計算する
 *
 * @return double エネルギー残量の減らすスピード
 */
double getVisibleRadiusRuducingRate(void);

/**
 * @brief アイテムの密度を計算する
 *
 * @return double アイテムの密度
 */
double getItemDensity(void);

/**
 * @brief 敵の生成間隔を計算する
 *
 * @return double 敵の生成間隔
 */
double getEnemySpawnIntervel(void);

#endif
