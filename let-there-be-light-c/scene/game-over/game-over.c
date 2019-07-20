/**
 * @file
 * @brief ゲームオーバーシーンのコントローラー
 */
#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "glut.h"

#include "config.h"
#include "state.h"

#include "scene/game-over/game-over.h"

#include "scene/scene.h"
#include "render/transition.h"
#include "scene/title/title.h"
#include "scene/level-title/level-title.h"
#include "scene/game/game.h"
#include "util/util.h"
/**
 * @file
 * @brief ゲームオーバーシーンのコントローラー
 */
#include "render/texture.h"
#include "render/engine.h"
#include "render/fx.h"

//! タイトルのフォントサイズ
#define CAPTION_FONT_SIZE           2.0
//! ヒントのフォントサイズ
#define HINT_FONT_SIZE              0.75
//! タイトルの文字が一個ずつ出るアニメーションの持続時間
#define CAPTION_ANIMATION_DURATION  150

//! A~Z ランダムな文字を生成する
#define RANDOM_CHAR           ((unsigned char)randomInt('A', 'Z'))

/**
 * @brief ゲームオーバーのシーンを初期化する
 */
static void initGameOver(void);
/**
 * @brief ゲームオーバーのシーンをレンダリングする
 */
static void renderGameOver(void);
/**
 * @brief ゲームオーバーのシーンから離れる時の処理
 */
static void destroyGameOver(void);

//! ゲームオーバーシーン
static Scene gameOver = {
  .init = initGameOver,
  .update = noop,
  .render = renderGameOver,
  .destroy = destroyGameOver,
};

Scene* gameOverScene = &gameOver;

//! タイトル
static const char* caption = "GAME OVER";

//! ヒント
static const char* hint = "[R]ETRY   [T]ITLE";

//! ヒントの透明度
static double hintAlpha = 0.0;
//! タイトルのアニメーションが終わった
static bool captionAnimationFinished = false;

// ============ Render Text ============ //

/**
 * @brief メッセージを画面の中央に出力する
 *
 * @param str メッセージ
 */
static void showMessage(const char* str) {
  size_t length = strlen(str);
  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  char* tmp = malloc(sizeof(char) * (length + 1));
  strcpy(tmp, str);

  int randomIdx = randomInt(0, (int)length - 1);

  if (tmp[randomIdx] == ' ') {
    randomIdx--;
  }

  // replace one letter randomly
  tmp[randomIdx] = RANDOM_CHAR;

  setTexParam(GL_MODULATE);
  glColor3d(0.6, 0.2, 0.2);
  renderText(tmp, (width - length * CAPTION_FONT_SIZE) / 2.0, (height - CAPTION_FONT_SIZE) / 2.0, CAPTION_FONT_SIZE);
  restoreDefaultTexParam();

  free(tmp);
}

/**
 * @brief ヒントをレンダリングする
 */
static void showHint(void) {
  size_t length = strlen(hint);
  double width = GameState.ortho.width;

  setTexParam(GL_MODULATE);
  glColor4d(1.0, 1.0, 1.0, hintAlpha);
  renderText(hint, (width - length * HINT_FONT_SIZE) / 2.0, 1.0, HINT_FONT_SIZE);
  restoreDefaultTexParam();
}

// ============ Hint Fading In ============ //

/**
 * @brief キーボードのハンドラ関数
 *
 * @param key 押されたキー
 * @param x   マウスの x 座標（使われていない）
 * @param y   マウスの y 座標（使われていない）
 */
static void keyboardHandler(unsigned char key, int x, int y) {
  UNUSED(x); UNUSED(y);

  if (key == 'R' || key == 'r') {
    levelTransition();
    destroyGameOver();
    return;
  }

  if (key == 'T' || key == 't') {
    initGameState();
    fxFadeOut(1000, titleScene);
    destroyGameOver();
    return;
  }
}

/**
 * @brief \ref hintFadeIn アニメーションをレンダリングする
 *
 * @param animation アニメーション
 */
static void hintFadeInRender(Animation* animation) {
  hintAlpha = (double)animation->currentFrame / animation->frameCount;
}

/**
 * @brief ヒントを次第に明るく出力するアニメーション
 */
static void hintFadeIn(void) {
  Animation* animation = createAnimation60FPS(300, 1);
  animation->update = hintFadeInRender;

  glutKeyboardFunc(keyboardHandler);
}

// ============ Caption Animation ============ //

/**
 * @brief タイトルのアニメーションが終わった後の処理
 *
 * @param _ アニメーション（使われていない）
 */
static void captionComplete(Animation* _) {
  UNUSED(_);

  hintFadeIn();
  captionAnimationFinished = true;
}

/**
 * @brief タイトルのアニメーションをレンダリングする
 *
 * @param animation アニメーション
 */
static void captionRender(Animation* animation) {
  size_t n = animation->nth;

  char* str = malloc(sizeof(char) * (n + 1));
  strncpy(str, caption, n - 1);
  str[n - 1] = caption[n - 1] == ' ' ? ' ' : RANDOM_CHAR;
  str[n] = '\0';

  showMessage(str);
  free(str);
}

// ============ Misc ============ //

static void initGameOver(void) {
  hintAlpha = 0.0;
  captionAnimationFinished = false;

  Animation* animation = createAnimation60FPS(CAPTION_ANIMATION_DURATION, (int)strlen(caption));

  animation->render = captionRender;
  animation->complete = captionComplete;
}

static void renderGameOver(void) {
  if (!captionAnimationFinished) {
    return;
  }

  showMessage(caption);
  showHint();
}

static void destroyGameOver(void) {
  glutKeyboardFunc(NULL);
}

