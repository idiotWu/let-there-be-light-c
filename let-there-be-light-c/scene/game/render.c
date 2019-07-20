/**
 * @file
 * @brief ゲームシーンのレンダリング
 */
#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <string.h>
#include <stdio.h>
#include "glut.h"

#include "config.h"
#include "state.h"

#include "scene/game/render.h"

#include "render/engine.h"
#include "render/texture.h"
#include "render/helper.h"
#include "maze/direction.h"
#include "maze/tile.h"
#include "util/list.h"
#include "util/util.h"

//! キャラクタスプライトを拡大する倍数
#define CHARACTER_SPRITE_SCALE      1.2
//! キャラクタスプライトの縦方向における基準位置
#define CHARACTER_SPRITE_BASELINE   0.3

//! 敵をゴーストとしてレンダリングする時の透明度
#define GHOST_ALPHA   0.1

//! エネルギーバーの長さ
#define ENERGY_BAR_WIDTH  (MAZE_SIZE / 4.0)

/**
 * @brief 迷路のタイルをレンダリングする
 */
static void renderTiles(void) {
  // render one more tile
  double r = GameState.visibleRadius + 1.0;

  size_t xMin = max(floor(GameState.player.x - r), 0);
  size_t xMax = min(ceil(GameState.player.x + r), MAZE_SIZE - 1);
  size_t yMin = max(floor(GameState.player.y - r), 0);
  size_t yMax = min(ceil(GameState.player.y + r), MAZE_SIZE - 1);

  for (size_t y = yMin; y <= yMax; y++) {
    for (size_t x = xMin; x <= xMax; x++) {
      Tile tile = GameState.maze[y][x];

      // path
      if (tile & TILE_OPEN) {
        int row, col;
        if (tile & TILE_SPOILED) {
          row = FROZEN_PATH_ROW;
          col = FROZEN_PATH_COL;
        } else {
          row = PATH_ROW;
          col = PATH_COL;
        }

        renderSprite(MISC_SPRITES, row, col, x, y, 1,  1);
      }

      // other items
      int row, col;

      if (tile & TILE_SHADOW) {
        row = SHADOW_ROW;
        col = SHADOW_COL;
      } else if(tile & TILE_COIN) {
        row = COIN_ROW;
        col = COIN_COL;
      } else if (tile & TILE_KERNEL) {
        row = FIRE_ROW;
        col = FIRE_COL;
      } else {
        continue;
      }

      renderSprite(MISC_SPRITES, row, col, x, y, 1, 1);
    }
  }
}

/**
 * @brief キャラクタ（敵，プレイヤー）をレンダリングする
 *
 * @param sprite    キャラクタスプライト
 * @param col       スプライトの列
 * @param x         キャラクタの位置の x 座標
 * @param y         キャラクタの位置の y 座標
 * @param direction キャラクタの向き
 */
static void renderCharacter(Sprite* sprite, int col,
                            double x, double y, Direction direction) {
  int row = CHARACTER_FRONT_ROW;

  switch (direction) {
    case DIR_UP:
      row = CHARACTER_BACK_ROW;
      break;

    case DIR_DOWN:
      row = CHARACTER_FRONT_ROW;
      break;

    case DIR_LEFT:
      row = CHARACTER_LEFT_ROW;
      break;

    case DIR_RIGHT:
      row = CHARACTER_RIGHT_ROW;
      break;

    default:
      break;
  }

  double offsetX = (1.0 - CHARACTER_SPRITE_SCALE) / 2.0;
  double offsetY = CHARACTER_SPRITE_BASELINE + offsetX;

  renderSprite(sprite, row, col,
               x + offsetX, y + offsetY,
               CHARACTER_SPRITE_SCALE, CHARACTER_SPRITE_SCALE);
}

/**
 * @brief プレイヤーをレンダリングする
 */
static void renderPlayer(void) {
  Player* player = &GameState.player;

  renderCharacter(player->spoiled ? PLAYER_SPRITES_SPOILED : PLAYER_SPRITES, player->spriteState, player->x, player->y, player->direction);
}

/**
 * @brief 敵をレンダリングする
 *
 * @param asGhost 敵をゴースト（半透明）としてレンダリングする
 */
static void renderEnemies(bool asGhost) {
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    Node* node = it.next(&it);
    Enemy* enemy = node->data;

    if (asGhost) {
      if (!enemy->activated) {
        continue;
      }

      setTexParam(GL_MODULATE);
      glColor4d(1.0, 1.0, 1.0, GHOST_ALPHA);
    }

    double ox = enemy->x + 0.5;
    double oy = enemy->y + 0.5;
    double scale = min(1.0, (double)enemy->remainSteps / ENEMY_MAX_STEPS + 0.2);

    glPushMatrix();
    glTranslated(ox, oy, 0.0);
    glScaled(scale, scale, 1.0);
    glTranslated(-ox, -oy, 0.0);

    renderCharacter(ENEMY_SPRITES, enemy->spriteState, enemy->x, enemy->y, enemy->direction);

    glPopMatrix();
    restoreDefaultTexParam();
  }
}

/**
 * @brief HUD をレンダリングする
 */
static void renderHUD(void) {
  glPushMatrix();
  glTranslated(0.0, MAZE_SIZE, 0.0);
  char buffer[32];
  size_t length;

  // energy bar
  setTexParam(GL_MODULATE);
  glPushMatrix();

  if (GameState.player.spoiled) {
    glTranslated(randomBetween(-0.2, 0.2), randomBetween(-0.2, 0.2), 0);
    glColor4d(0.2, 0.5, 1.0, 1.0);
  } else {
    glColor4d(1.0, 1.0, 1.0, 1.0);
  }

  sprintf(buffer, "E");
  length = strlen(buffer);
  renderText(buffer, 0, 0, 1);

  renderEnergyBar(clamp(GameState.visibleRadius / MAX_VISIBLE_RADIUS, 0.0, 1.0),
                  length + 0.25, 0, ENERGY_BAR_WIDTH, 1);

  glPopMatrix();
  restoreDefaultTexParam();

  // remain item count
  sprintf(buffer, "%03d", GameState.remainItem);
  length = strlen(buffer);
  renderText(buffer, MAZE_SIZE - length, 0, 1);
  renderSprite(MISC_SPRITES, COIN_BIG_ROW, COIN_BIG_COL,
               MAZE_SIZE - length - 1.25, 0, 1, 1);

  glPopMatrix();
}

void renderGame(void) {
  // visible world
  double worldX = GameState.player.x - GameState.visibleRadius + 0.5;
  double worldY = GameState.player.y - GameState.visibleRadius + 0.5;
  double worldSize = GameState.visibleRadius * 2.0;

  glEnable(GL_STENCIL_TEST);
  clipRect(worldX, worldY, worldSize, worldSize);

  renderTiles();
  renderPlayer();
  renderEnemies(false);

  glDisable(GL_STENCIL_TEST);

  renderFog(worldX, worldY, worldSize, worldSize);
  // render enemies as ghost
  renderEnemies(true);

  renderHUD();
}

