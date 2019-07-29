/**
 * @file
 * @brief タイムラインに基づいたアニメーションエンジン ＜ヘッダファイル＞
 *
 * @details エンジンの仕様：
 * ```
 *       t
 * ------+------------------------------------------> timeline
 * +-----|-+
 * |   A | | <- animation object
 * +-----|-+
 *    +--|------+
 *    |  | A    |
 *    +--|------+
 *  +----|+-----+-----+-----+-----+
 *  |  A ||  A  |  A  |  A  |  A  | <- repeating animation
 *  +----|+-----+-----+-----+-----+
 *       |
 *    render(t)
 * ```
 *
 * エンジンの使用例：
 * ```c
 * void renderFn(Animation* animation);
 * void updateFn(Animation* animation);
 * void completeFn(Animation* animation);
 *
 * void initAnimation(void) {
 *   Animation* animation = createAnimaion(10, 1000, ANIMATION_INFINITY);
 *   animation->render = renderFn;
 *   animation->update = updateFn;
 *   animation->complete = completeFn;
 *
 *   animation->target = someObj;
 *   animation->from = fromVal;
 *   animation->delta = deltaVal;
 *
 *   animation->clean = ANIMATION_CLEAN_FROM | ANIMATION_CLEAN_DELTA;
 * }
 * ```
 */
#ifndef ENGINE_H
#define ENGINE_H

#include <limits.h>
#include <stdbool.h>

/**
 * @brief アニメーションのリフレッシュ間隔
 */
#define ANIMATION_REFRESH_INTERVAL (1000.0 / 60.0)

/**
 * @brief 1 回のみのアニメーション
 */
#define ANIMATION_ONCE 1
/**
 * @brief 無限に繰り返すアニメーション
 */
#define ANIMATION_INFINITY INT_MAX

/**
 * @brief アニメーションの後始末のフラグ
 */
typedef enum AnimationCleanFlag {
  //! 何もしない
  ANIMATION_CLEAN_NONE = 0,

  //! `animation.target` のメモリ領域を解放する
  ANIMATION_CLEAN_TARGET = 1 << 0,
  //! `animation.from` のメモリ領域を解放する
  ANIMATION_CLEAN_FROM = 1 << 1,
  //! `animation.delta` のメモリ領域を解放する
  ANIMATION_CLEAN_DELTA = 1 << 2,

  //! 上記の3つのメモリ領域をすべて解放する
  ANIMATION_CLEAN_ALL = ANIMATION_CLEAN_TARGET | ANIMATION_CLEAN_FROM | ANIMATION_CLEAN_DELTA,
} AnimationCleanFlag;

/**
 * @brief アニメーションオブジェクト
 */
typedef struct Animation {
  //! ターゲットを記録するポイント変数
  void* target;
  //! 始まりの値を記録するポイント変数
  void* from;
  //! 変化量を記録するポイント変数
  void* delta;

  //! 現在のフレーム
  int currentFrame;
  //! 総計フレーム数
  int frameCount;

  /**
   * @private
   * @brief フレームを更新してから経過したフレーム数
   */
  int elapsed;
  /**
   * @private
   * @brief フレーム更新の間隔
   */
  int interval;

  /**
   * @brief 繰り返しているアニメーションにおいて，現在の回数
   */
  int nth;
  /**
   * @brief 繰り返す回数
   */
  int repeat;

  /**
   * @private
   * @brief 次のイテレーションでこのアニメーションを削除する
   */
  bool deleteInNextIteration;

  /**
   * @brief アニメーションの後始末フラグ
   */
  AnimationCleanFlag cleanFlag;

  /**
   * @brief アニメーションのレンダリング関数
   */
  void (*render)(struct Animation*);
  /**
   * @brief アニメーションがアップデートするときのコールバック関数
   */
  void (*update)(struct Animation*);
  /**
   * @brief アニメーションが終わったときのコールバック関数
   */
  void (*complete)(struct Animation*);
} Animation;

/**
 * @brief 遅らせて実行する関数
 */
typedef void (*DelayCallback)(void*);

/**
 * @brief アニメーションを生成し，メインタイムラインに追加する
 *
 * @details アニメーションの仕様：
 * ```
 *  3 frames/period non-repeat:
 *   begin <-------|-------|-------> end
 *             1       2       3
 *
 *  3 frames/period * 2 repeats:
 *    begin <---|---|---><---|---|---> end
 *            1   2   3    1   2   3
 * ```
 *
 * @param frames   フレーム数
 * @param duration アニメーションの持続時間
 * @param repeat   繰り返す回数
 *
 * @return Animation* アニメーション
 */

Animation* createAnimation(int frames, double duration, int repeat);

/**
 * @brief 60 FPS のアニメーションを生成する
 *
 * @param duration アニメーションの持続時間
 * @param repeat   繰り返す回数
 *
 * @return Animation* アニメーション
 */
Animation* createAnimation60FPS(double duration, int repeat);

/**
 * @brief 指定のアニメーションをキャンセルする
 *
 * @param animation キャンセルしたいアニメーション
 */
void cancelAnimation(Animation* animation);

/**
 * @brief アニメーションエンジンを次のフレームに動かす
 */
void engineNextFrame(void);

/**
 * @brief エンジンをレンダリングする
 */
void engineRender(void);

/**
 * @brief アニメーションを利用し，関数を遅らせて実行する
 *
 * @param duration 遅らせる時間
 * @param callback 目標関数
 * @param data     目標関数の引数に渡すデータ
 *
 * @return Animation* 目標関数を遅らせるアニメーション
 */
Animation* delay(double duration, DelayCallback callback, void* data);

#endif
