/**
 * @file
 * @brief キーボードの処理 ＜ヘッダファイル＞
 */
#ifndef GAME_KEYBOARD_H
#define GAME_KEYBOARD_H

#include "maze/direction.h"

/**
 * @brief キーボードイベントハンドラ関数を有効にする
 */
void bindGameKeyboardHandlers(void);
/**
 * @brief キーボードイベントハンドラ関数を無効にする
 */
void removeGameKeyboardHandlers(void);

/**
 * @brief 押している方向キーを読み取る
 *
 * @return Direction 押している方向（複数の場合がある）
 */
Direction getPressedKeys(void);

#endif
