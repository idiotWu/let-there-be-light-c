/**
 * @file
 * @brief キーボードの処理
 */
#include "glut.h"

#include "scene/game/keyboard.h"

#include "util/util.h"
#include "maze/direction.h"

static Direction keyPressed = DIR_NONE;

/**
 * @brief 方向キーのコードを方向に変換する
 *
 * @param key 方向キーのコード
 *
 * @return Direction 方向
 */
static Direction keyToDirection(int key) {
  switch (key) {
    case GLUT_KEY_UP:
      return DIR_UP;

    case GLUT_KEY_RIGHT:
      return DIR_RIGHT;

    case GLUT_KEY_DOWN:
      return DIR_DOWN;

    case GLUT_KEY_LEFT:
      return DIR_LEFT;

    default:
      return DIR_NONE;
  }
}

/**
 * @brief キーが押された時のハンドラ関数
 *
 * @param key 押されたキー
 * @param x   マウスの x 座標（使われていない）
 * @param y   マウスの y 座標（使われていない）
 */
static void keydownHandler(int key, int x, int y) {
  UNUSED(x); UNUSED(y);
  setBits(keyPressed, keyToDirection(key));
}

/**
 * @brief キーが離された時のハンドラ関数
 *
 * @param key 離されたキー
 * @param x   マウスの x 座標（使われていない）
 * @param y   マウスの y 座標（使われていない）
 */
static void keyupHandler(int key, int x, int y) {
  UNUSED(x); UNUSED(y);
  clearBits(keyPressed, keyToDirection(key));
}

void bindGameKeyboardHandlers(void) {
  keyPressed = DIR_NONE;

  glutSpecialFunc(keydownHandler);
  glutSpecialUpFunc(keyupHandler);
}

void removeGameKeyboardHandlers(void) {
  keyPressed = DIR_NONE;

  glutSpecialFunc(NULL);
  glutSpecialUpFunc(NULL);
}

Direction getPressedKeys(void) {
  return keyPressed;
}
