/**
 * @file
 * @brief テクスチャ ＜ヘッダファイル＞
 */
#ifndef TEXTURE_H
#define TEXTURE_H

#include "glut.h"

//! キャラクタスが前に向いている状態を表すスプライトの行
#define CHARACTER_FRONT_ROW 0
//! キャラクタスが後ろに向いている状態を表す行
#define CHARACTER_BACK_ROW  1
//! キャラクタスが左に向いている状態を表す行
#define CHARACTER_LEFT_ROW  2
//! キャラクタスが右に向いている状態を表す行
#define CHARACTER_RIGHT_ROW 3

//! 敵が眠っている状態を表す列
#define ENEMY_INACTIVE_COL 3

//! 地面を表す行
#define PATH_ROW 0
//! 地面を表す列
#define PATH_COL 0

//! 陰を表す行
#define SHADOW_ROW 0
//! 陰を表す列
#define SHADOW_COL 1

//! 凍った地面を表す行
#define FROZEN_PATH_ROW 1
//! 凍った地面を表す列
#define FROZEN_PATH_COL 0

//! 氷の矢を表す行
#define ICE_SPIKE_ROW 1
//! 氷の矢を表す列
#define ICE_SPIKE_COL 1

//! 火の玉を表す行
#define FIRE_ROW 2
//! 火の玉を表す列
#define FIRE_COL 0

//! コインを表す行
#define COIN_ROW 2
//! コインを表す列
#define COIN_COL 1

//! 黒いコインを表す行
#define COIN_DARK_ROW 3
//! 黒いコインを表す列
#define COIN_DARK_COL 0

//! 大きいコインを表す行
#define COIN_BIG_ROW 3
//! 大きいコインを表す列
#define COIN_BIG_COL 1

//! 爆発エフェクトの行
#define FX_EXPLODE_ROW 0
//! 地面が凍てつくエフェクトの行
#define FX_ICE_SPLIT_ROW 1
//! 敵が生まれるエフェクトの行
#define FX_ENEMY_SPAWN_ROW 2
//! 敵が蘇るエフェクトの行
#define FX_ENEMY_REVIVE_ROW 3
//! スモークエフェクトの行
#define FX_SMOKE_ROW 4

/**
 * @brief スプライトのオブジェクト
 */
typedef struct Sprite {
  //! スプライトのテクスチャ
  GLuint texture;
  //! スプライトの行数
  GLuint rows;
  //! スプライトの列数
  GLuint cols;
} Sprite;

//! プレイヤーのスプライト
extern Sprite* PLAYER_SPRITES;
//! プレイヤーが凍える状態のスプライト
extern Sprite* PLAYER_SPRITES_SPOILED;
//! 敵のスプライト
extern Sprite* ENEMY_SPRITES;
//! アイテムとか，様々のものを含むスプライト
extern Sprite* MISC_SPRITES;
//! エフェクトのスプライト
extern Sprite* FX_SPRITES;
//! フォントのスプライト
extern Sprite* FONT_SPRITES;
//! エネルギーバーのスプライト
extern Sprite* ENERGY_BAR_SPRITES;

/**
 * @brief テクスチャのパラメータを設定する
 *
 * @param param パラメータ
 */
void setTexParam(GLfloat param);

/**
 * @brief テクスチャのパラメータを初期値に戻す
 */
void restoreDefaultTexParam(void);

/**
 * @brief スプライトをレンダリングする
 *
 * @param sprite レンダリングしたいスプライト
 * @param row    レンダリングしたい行
 * @param col    レンダリングしたい列
 * @param dx     出力場所の x 座標
 * @param dy     出力場所の y 座標
 * @param dw     出力の長さ
 * @param dh     出力の高さ
 */
void renderSprite(Sprite* sprite,
                  uint32_t row, uint32_t col,
                  double dx, double dy,
                  double dw, double dh);

/**
 * @brief 霧（見える範囲）をレンダリングする
 *
 * @param dx 出力場所の x 座標
 * @param dy 出力場所の y 座標
 * @param dw 出力の長さ
 * @param dh 出力の高さ
 */
void renderFog(double dx, double dy,
               double dw, double dh);

/**
 * @brief エネルギーバーをレンダリングする
 *
 * @param percent エネルギーの残量（%）
 * @param dx      出力場所の x 座標
 * @param dy      出力場所の y 座標
 * @param dw      出力の長さ
 * @param dh      出力の高さ
 */
void renderEnergyBar(double percent,
                     double dx, double dy,
                     double dw, double dh);

/**
 * @brief 文字列をレンダリングする
 *
 * @param str      レンダリングしたい文字列
 * @param x        出力場所の x 座標
 * @param y        出力場所の y 座標
 * @param fontSize 文字サイズ
 */
void renderText(const char* str,
                double x, double y,
                double fontSize);

/**
 * @brief テクスチャを読み取る
 */
void loadTextures(void);

#endif
