/**
 * @file
 * @brief 様々なエフェクト ＜ヘッダファイル＞
 */
#ifndef EXPR_FX_H
#define EXPR_FX_H

#include "render/engine.h"
#include "scene/scene.h"

/**
 * @brief 次第に明るくなるアニメーション
 *
 * @param duration アニメーションの持続時間
 * @param toScene  アニメーションが始まるに入るシーン
 */
void fxFadeIn(double duration, Scene* toScene);

/**
 * @brief 次第に暗くなるアニメーション
 *
 * @param duration アニメーションの持続時間
 * @param toScene  アニメーションが終わった後に入るシーン
 */
void fxFadeOut(double duration, Scene* toScene);

/**
 * @brief 爆発のアニメーション
 *
 * @param spriteRow 爆発スプライトの行
 * @param x         爆発が発生する場所の x 座標
 * @param y         爆発が発生する場所の y 座標
 * @param size      爆発のサイズ
 */
void fxExplode(int spriteRow, double x, double y, double size);

/**
 * @brief 指定のタイルに標準サイズで爆発するアニメーション
 *
 * @param spriteRow 爆発スプライトの行
 * @param x         爆発が発生するタイルの x 座標
 * @param y         爆発が発生するタイルの y 座標
 */
void fxExplodeAtTile(int spriteRow, int x, int y);

/**
 * @brief 水が迷路全体に流れていくようなアニメーション
 *
 * @param x エフェクトが発生する場所の x 座標
 * @param y エフェクトが発生する場所の y 座標
 */
void fxFlood(int x, int y);

#endif
