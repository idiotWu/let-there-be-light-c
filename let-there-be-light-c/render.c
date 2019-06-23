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
#include "engine.h"
#include "texture.h"

#define PLAYER_SPRITE_SCALE     1.2
#define PLSYER_SPRITE_BASELINE  0.3

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
        renderSprite(MISC_SPRITES, PATH_ROW, PATH_COL, x, y, 1,  1);
      }

      // other items
      int row, col;

      if (tile == TILE_SHADOW) {
        row = SHADOW_ROW;
        col = SHADOW_COL;
      } else if(tile == TILE_COIN) {
        row = COIN_ROW;
        col = COIN_COL;
      } else if (tile == TILE_KERNEL) {
        row = FIRE_ROW;
        col = FIRE_COL;
      } else {
        continue;
      }

      renderSprite(MISC_SPRITES, row, col, x, y, 1, 1);
    }
  }
}

static void renderPlayer(void) {
  int row = PLAYER_FRONT_ROW;

  switch (GameState.player.direction) {
    case DIR_UP:
      row = PLAYER_BACK_ROW;
      break;

    case DIR_DOWN:
      row = PLAYER_FRONT_ROW;
      break;

    case DIR_LEFT:
      row = PLAYER_LEFT_ROW;
      break;

    case DIR_RIGHT:
      row = PLAYER_RIGHT_ROW;
      break;

    default:
      break;
  }

  double offsetX = (1.0 - PLAYER_SPRITE_SCALE) / 2.0;
  double offsetY = PLSYER_SPRITE_BASELINE + offsetX;

  renderSprite(PLAYER_SPRITES,
               row, GameState.player.state,
               GameState.player.x + offsetX, GameState.player.y + offsetY,
               PLAYER_SPRITE_SCALE, PLAYER_SPRITE_SCALE);
}

static void renderHUD(void) {
  glPushMatrix();
  glTranslated(0.0, MAZE_SIZE, 0.0);
  char buffer[32];
  size_t length;

  // energy bar
  sprintf(buffer, "E");
  length = strlen(buffer);
  renderText(buffer, 0, 0, 1);

  renderEnergyBar(clamp(GameState.visibleRadius / MAX_VISIBLE_RADIUS, 0.0, 1.0), length, 0, ENERGY_BAR_WIDTH, 1);

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

  glDisable(GL_STENCIL_TEST);
  renderFog(worldX, worldY, worldSize, worldSize);

  renderHUD();
}
