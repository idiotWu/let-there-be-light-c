/**
 * @file
 * @brief シーンの切り替えるアニメーション ＜ヘッダファイル＞
 */
#ifndef TRANSITION_H
#define TRANSITION_H

#include "scene/scene.h"
#include "render/engine.h"

/**
 * @brief スクリーン上に 2 つの三角形が徐々に閉じていくアニメーション
 *
 * @param duration アニメーションの持続時間
 *
 * @return Animation* アニメーション
 */
Animation* transitionIn(double duration);
/**
 * @brief スクリーン上に 2 つの三角形が徐々に開いていくアニメーション
 *
 * @param duration アニメーションの持続時間
 *
 * @return Animation* アニメーション
 */
Animation* transitionOut(double duration);

/**
 * @brief 一連のシーンを切り替えていくアニメーション
 *
 * @details 処理手順は：
 * ```
 *             [duration]           [duration]
 * current scene ------> midway scene ------> target scene
 *                       [   rest   ]
 * ```
 *
 * @param duration アニメーションの持続時間
 * @param target   目標のシーン
 * @param rest     途中での休憩時間（途中のシーンの表示時間）
 * @param midway   途中のシーン
 */
void transitionQueue(double duration, Scene* target, double rest, Scene* midway);

/**
 * @brief 次のステージに切り替える
 */
void levelTransition(void);

#endif
