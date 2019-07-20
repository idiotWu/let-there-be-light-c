/**
 * @file
 * @brief 様々なエフェクト
 */
#include <stdlib.h>
#include "glut.h"

#include "config.h"
#include "state.h"

#include "render/fx.h"

#include "render/engine.h"
#include "render/texture.h"
#include "scene/scene.h"
#include "maze/tile.h"
#include "maze/floodfill.h"
#include "util/list.h"
#include "util/util.h"

//! 爆発アニメーションの持続時間
#define FX_EXPLODE_DURATION       300
//! 標準の爆発サイズ
#define FX_EXPLODE_STD_SIZE       1.2
//! 洪水アニメーションの持続時間
#define FX_FLOOD_DURADION         600
//! 洪水アニメーションが次の場所に流れていく時刻
#define FX_FLOOD_INFLEXION_POINT  0.1

// ============ Fade In Effect ============ //

/**
 * @brief \ref fadeIn アニメーションをレンダリングする
 *
 * @param animation fadeIn アニメーション
 */
static void fxFadeInRender(Animation* animation) {
  double alpha = 1.0 - (double)animation->currentFrame / animation->frameCount;

  glColor4d(0.0, 0.0, 0.0, alpha);
  glRectd(GameState.ortho.left, GameState.ortho.bottom,
          GameState.ortho.right, GameState.ortho.top);
}

void fxFadeIn(double duration, Scene* toScene) {
  switchScene(toScene);

  Animation* animation = createAnimation60FPS(duration, 1);
  animation->render = fxFadeInRender;
}

// ============ Fade Out Effect ============ //

/**
 * @brief \ref fadeOut アニメーションが終わった後の処理（シーンの切り替え）
 *
 * @param animation fadeOut アニメーション
 */
static void fxFadeOutComplete(Animation* animation) {
  Scene* toScene = animation->target;

  switchScene(toScene);
}

/**
 * @brief \ref fadeOut アニメーションをレンダリングする
 *
 * @param animation fadeOut アニメーション
 */
static void fxFadeOutRender(Animation* animation) {
  double alpha = (double)animation->currentFrame / animation->frameCount;

  glColor4d(0.0, 0.0, 0.0, alpha);
  glRectd(GameState.ortho.left, GameState.ortho.bottom,
          GameState.ortho.right, GameState.ortho.top);
}

void fxFadeOut(double duration, Scene* toScene) {
  Animation* animation = createAnimation60FPS(duration, 1);
  animation->render = fxFadeOutRender;
  animation->complete = fxFadeOutComplete;
  animation->target = toScene;
  animation->cleanFlag = ANIMATION_CLEAN_NONE;
}

// ============ Explode Effect ============ //

/**
 * @brief 爆発アニメーションをレンダリングする
 *
 * @param animation 爆発アニメーション
 */
static void fxExplodeRender(Animation* animation) {
  int* row = animation->target;
  vec2d* start = animation->from;
  double* size = animation->delta;

  glPushMatrix();
  glTranslated(start->x, start->y, 0.0);
  glScaled(*size, *size, 1.0);
  glTranslated(-start->x, -start->y, 0.0);

   // centering sprite
  renderSprite(FX_SPRITES, *row, animation->currentFrame - 1,
               start->x - 0.5, start->y - 0.5, 1.0, 1.0);
  glPopMatrix();
}

void fxExplode(int spriteRow, double x, double y, double size) {
  Animation* animation = createAnimation(FX_SPRITES->cols, FX_EXPLODE_DURATION, 1);

  vec2d* start = malloc(sizeof(vec2d));
  start->x = x;
  start->y = y;

  int* row = malloc(sizeof(int));
  *row = spriteRow;

  double* s = malloc(sizeof(double));
  *s = size;

  animation->target = row;
  animation->from = start;
  animation->delta = s;
  animation->cleanFlag = ANIMATION_CLEAN_TARGET | ANIMATION_CLEAN_FROM | ANIMATION_CLEAN_DELTA;

  animation->render = fxExplodeRender;
}

void fxExplodeAtTile(int spriteRow, int x, int y) {
  fxExplode(spriteRow, x + 0.5, y + 0.5, FX_EXPLODE_STD_SIZE);
}

// ============ Flood Effect ============ //

/**
 * @brief Flood の状態を記録するオブジェクト
 */
typedef struct FxFloodRecord {
  //! Flood の状態
  FloodState* state;
  //! Flood の先頭
  List* frontiers;
} FxFloodRecord;

/**
 * @brief \ref fxFlood アニメーションをレンダリングする
 *
 * @param animation Flood アニメーション
 */
static void fxFloodRender(Animation* animation);
/**
 * @brief \ref fxFlood アニメーションを更新する
 *
 * @param animation Flood アニメーション
 */
static void fxFloodUpdate(Animation* animation);
/**
 * @brief \ref fxFlood アニメーションが終わった後の処理
 *
 * @param animation Flood アニメーション
 */
static void fxFloodFinish(Animation* animation);

/**
 * @brief 凍った地面をクリアする
 *
 * @param x 凍った地面の x 座標
 * @param y 凍った地面の y 座標
 */
static void clearSpoiledTile(int x, int y) {
  if (GameState.maze[y][x] & TILE_SPOILED) {
    clearBits(GameState.maze[y][x], TILE_SPOILED);
    fxExplodeAtTile(FX_SMOKE_ROW, x + 0.5, y + 0.5);
  }
}

/**
 * @brief Flood を 1 回進める
 *
 * @param state Flood の状態
 */
static void fxFloodNext(FloodState* state) {
  if (state->finished) {
    floodDestory(state);
    return;
  }

  floodForward(state);

  FxFloodRecord* record = malloc(sizeof(FxFloodRecord));

  record->state = state;
  record->frontiers = listClone(state->frontiers, sizeof(vec2i));

  Animation* animation = createAnimation60FPS(FX_FLOOD_DURADION, 1);
  animation->target = record;
  animation->render = fxFloodRender;
  animation->update = fxFloodUpdate;
  animation->complete = fxFloodFinish;
}

/**
 * @brief Flood の先頭をレンダリングする（先頭にあるアイテムを示す）
 *
 * @param record Flood の状態
 * @param scale  アイテムの拡大倍数
 * @param alpha  透明度
 */
static void renderFrontiers(FxFloodRecord* record, double scale, double alpha) {
  ListIterator it = createListIterator(record->frontiers);

  // enable alpha blend
  setTexParam(GL_MODULATE);
  glColor4d(alpha, alpha, alpha, alpha);

  while (!it.done) {
    Node* node = it.next(&it);
    vec2i* frontier = node->data;
    int x = frontier->x;
    int y = frontier->y;

    double ox = x + 0.5;
    double oy = y + 0.5;

    clearSpoiledTile(x, y);

    int row, col;

    Tile tile = GameState.maze[y][x];

    if (tile & TILE_COIN) {
      row = COIN_ROW;
      col = COIN_COL;
    } else if (tile & TILE_KERNEL) {
      row = FIRE_ROW;
      col = FIRE_COL;
    } else {
      row = COIN_DARK_ROW;
      col = COIN_DARK_COL;
    }

    glPushMatrix();
      glTranslated(ox, oy, 0.0);
      glScaled(scale, scale, 1.0);
      glTranslated(-ox, -oy, 0.0);
      renderSprite(MISC_SPRITES, row, col, x, y, 1, 1);
    glPopMatrix();
  }

  restoreDefaultTexParam();
}

static void fxFloodUpdate(Animation* animation) {
  FxFloodRecord* record = animation->target;
  uint16_t throttle = FX_FLOOD_INFLEXION_POINT * animation->frameCount;

  if (animation->currentFrame >= throttle) {
    fxFloodNext(record->state);
    animation->update = NULL; // remove callback
  }
}

static void fxFloodRender(Animation* animation) {
  FxFloodRecord* record = animation->target;
  uint16_t throttle = FX_FLOOD_INFLEXION_POINT * animation->frameCount;

  double percent;

  if (animation->currentFrame < throttle) {
    percent =(double)animation->currentFrame / throttle;
  } else {
    percent = 1.0 - (double)(animation->currentFrame - throttle) / (animation->frameCount - throttle);
  }

  renderFrontiers(record, percent, percent);
}

static void fxFloodFinish(Animation* animation) {
  FxFloodRecord* record = animation->target;
  listDestory(record->frontiers);
}

void fxFlood(int x, int y) {
  FloodState* state = floodGenerate(GameState.maze, x, y);

  fxExplodeAtTile(FX_EXPLODE_ROW, x, y);

  fxFloodNext(state);

  clearSpoiledTile(x, y);
}
