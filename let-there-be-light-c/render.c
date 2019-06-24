#include <math.h>
#include <string.h>
#include <stdio.h>
#include "glut.h"

#include "config.h"
#include "render.h"
#include "state.h"
#include "direction.h"
#include "tile.h"
#include "util.h"
#include "list.h"
#include "engine.h"
#include "texture.h"

#define CHARACTER_SPRITE_SCALE     1.2
#define CHARACTER_SPRITE_BASELINE  0.3

#define ENERGY_BAR_WIDTH        (MAZE_SIZE / 4.0)

static void clipRect(double x, double y, double width, double height) {
  // enable writing to the stencil buffer
  glStencilMask(0xff);
  // Clear stencil buffer
  glClear(GL_STENCIL_BUFFER_BIT);
  // discard pixels out of stencil area
  glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
  // all fragments should update the stencil buffer
  glStencilFunc(GL_ALWAYS, 1, 0xff);

  // draw a rectangle into stencil
  glColor4d(0.0, 0.0, 0.0, 0.0);
  glRectd(x, y, x + width, y + height);

  // disable writing to the stencil buffer
  glStencilMask(0);

  // only keeps pixels inside the rectangle area
  glStencilFunc(GL_EQUAL, 1, 0xff);
}

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

static void renderPlayer(void) {
  Player* player = &GameState.player;

  renderCharacter(player->spoiled ? PLAYER_SPRITES_SPOILED : PLAYER_SPRITES, player->spriteState, player->x, player->y, player->direction);
}

static void renderEnemies(void) {
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    EnemyNode* node = it.next(&it);
    Enemy* enemy = node->data;

    renderCharacter(ENEMY_SPRITES, enemy->spriteState, enemy->x, enemy->y, enemy->direction);
  }
}

static void renderHUD(void) {
  glPushMatrix();
  glTranslated(0.0, MAZE_SIZE, 0.0);
  char buffer[32];
  size_t length;

  // energy bar
  setTexParam(GL_MODULATE);

  if (GameState.player.spoiled) {
    glColor4d(0.0, 0.5, 1.0, 1.0);
  } else {
    glColor4d(1.0, 1.0, 1.0, 1.0);
  }

  sprintf(buffer, "E");
  length = strlen(buffer);
  renderText(buffer, 0, 0, 1);

  renderEnergyBar(clamp(GameState.visibleRadius / MAX_VISIBLE_RADIUS, 0.0, 1.0), length, 0, ENERGY_BAR_WIDTH, 1);

  restoreDefaultTexParam();

  // remain item count
  sprintf(buffer, "%03d", GameState.remainItem);
  length = strlen(buffer);
  renderText(buffer, MAZE_SIZE - length, 0, 1);
  renderSprite(MISC_SPRITES, COIN_BIG_ROW, COIN_BIG_COL, MAZE_SIZE - length - 1, 0, 1, 1);

  glPopMatrix();
}

void renderWorld(void) {
  // visible world
  double worldX = GameState.player.x - GameState.visibleRadius + 0.5;
  double worldY = GameState.player.y - GameState.visibleRadius + 0.5;
  double worldSize = GameState.visibleRadius * 2.0;

  glEnable(GL_STENCIL_TEST);
  clipRect(worldX, worldY, worldSize, worldSize);

  renderTiles();
  renderPlayer();
  renderEnemies();

  glDisable(GL_STENCIL_TEST);
  renderFog(worldX, worldY, worldSize, worldSize);

  renderHUD();
}
