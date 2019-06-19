#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "glut.h"

#include "tile.h"
#include "maze.h"
#include "state.h"
#include "game.h"
#include "config.h"
#include "engine.h"
#include "util.h"
#include "texture.h"
#include "direction.h"
#include "expr-fx.h"

#define GAME_TITLE "光よ、あれ！"

#define UNUSED(x) (void)(x)

#define ENABLE_CLIPPING

void drawFog(bool asStencil) {
  double x = GameState.player.x - GameState.visibleRadius;
  double y = GameState.player.y - GameState.visibleRadius;

  double size = GameState.visibleRadius * 2.0 + 1.0;

  if (asStencil) {
    glColor4d(0.0, 0.0, 0.0, 0.0);
    glRectd(x, y, x + size, y + size);
  } else {
    renderFog(x, y, size, size);
  }
}

void drawTiles() {
  glEnable(GL_STENCIL_TEST);
#ifdef ENABLE_CLIPPING
  // enable writing to the stencil buffer
  glStencilMask(0xff);
  // Clear stencil buffer
  glClear(GL_STENCIL_BUFFER_BIT);
  // discard pixels out of stencil area
  glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
  // all fragments should update the stencil buffer
  glStencilFunc(GL_ALWAYS, 1, 0xff);

  // draw fog as the stencil
  drawFog(true);

  // disable writing to the stencil buffer
  glStencilMask(0);

  // draw tiles
  // only keeps pixels inside the fog area
  glStencilFunc(GL_EQUAL, 1, 0xff);

  // render one more tile
  double r = GameState.visibleRadius + 1.0;

  size_t xMin = max(floor(GameState.player.x - r), 0);
  size_t xMax = min(ceil(GameState.player.x + r), MAZE_SIZE - 1);
  size_t yMin = max(floor(GameState.player.y - r), 0);
  size_t yMax = min(ceil(GameState.player.y + r), MAZE_SIZE - 1);

  for (size_t y = yMin; y <= yMax; y++) {
    for (size_t x = xMin; x <= xMax; x++) {
#else
  for (size_t y = 0; y < MAZE_SIZE; y++) {
    for (size_t x = 0; x < MAZE_SIZE; x++) {
#endif
      Tile tile = GameState.map[y][x];

      // path
      if (tile & TILE_OPEN) {
        renderSprite(MISC_SPRITES, PATH_ROW, PATH_COL, x, y, 1,  1);
      }

      // other items
      int row, col;

      if (tile == TILE_SHADOW) {
        row = SHADOW_ROW;
        col = SHADOW_COL;
      } else {
        continue;
      }

      renderSprite(MISC_SPRITES, row, col, x, y, 1, 1);
    }
  }

  glDisable(GL_STENCIL_TEST);
}

void drawPlayer() {
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

  renderSprite(PLAYER_SPRITES,
               row, 0,
               GameState.player.x - 0.1, GameState.player.y + 0.3,
               1.2, 1.2);
}

void renderWorld() {
  drawTiles();
  drawPlayer();

#ifdef ENABLE_CLIPPING
  drawFog(false);
#endif
}

void keyboardHandler(unsigned char key, int x, int y) {
  UNUSED(x);
  UNUSED(y);

  if (key == ' ') {
    fxGen();
  }
}

void init(void) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  initTextures();
  buildWorld(0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  renderWorld();

  engineRender();

  glutSwapBuffers();
}

void update(int _) {
  static int lastT = 0;
  int nowT = glutGet(GLUT_ELAPSED_TIME);

  printf("elapsed: %d\n", nowT - lastT);

  lastT = nowT;

  glutTimerFunc(ANIMATION_60_FPS, update, _);

  engineNextFrame();

#ifdef __APPLE__
  // `glutPostRedisplay()` blocks the next timer on mac
  display();
#else
  glutPostRedisplay();
#endif
}

void reshape(int w, int h) {
  ClientRect* vp = &GameState.viewport;
  ClientRect* ortho = &GameState.ortho;

  double ratio = (double)MAZE_SIZE / (MAZE_SIZE + HUD_HEIGHT);
  double size = clamp(min(/* w */ h * ratio, /* h */ w / ratio),
                      0, min(w, h));

  vp->width = size;
  vp->height = size / ratio;
  vp->left = (w - vp->width) / 2;
  vp->bottom = (h - vp->height) / 2;
  vp->top = vp->bottom + vp->height;
  vp->right = vp->left + vp->width;

  glViewport(vp->left, vp->bottom, vp->width, vp->height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  ortho->top = MAZE_SIZE + HUD_HEIGHT;
  ortho->right = MAZE_SIZE;
  ortho->bottom = 0;
  ortho->left = 0;
  ortho->width = ortho->right - ortho->left;
  ortho->height = ortho->top - ortho->bottom;

  gluOrtho2D(ortho->left, ortho->right, ortho->bottom, ortho->top);

  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
  glutInitWindowSize(800, 800);
  glutCreateWindow(GAME_TITLE);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboardHandler);
  init();
  update(0);
  glutMainLoop();

  return EXIT_SUCCESS;
}
