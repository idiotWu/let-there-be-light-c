/**
 * @file
 * @brief シーンコントローラーのプロトタイプ（ヘッダファイル）
 */
#ifndef SCENE_H
#define SCENE_H

//! シーンコントローラー
typedef struct Scene {
  //! シーンを初期化するメソッド
  void (*init)(void);
  //! シーンを更新するメソッド
  void (*update)(void);
  //! シーンをレンダリングするメソッド
  void (*render)(void);
  //! シーンから離れる時の処理
  void (*destroy)(void);
} Scene;

/**
 * @brief シーンを切り替える
 *
 * @param nextScene 次のシーン
 */
void switchScene(Scene* nextScene);

#endif
