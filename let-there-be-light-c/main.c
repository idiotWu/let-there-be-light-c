#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

#define UNUSED(x) (void)(x)

#define ENABLE_CLIPPING

void clipMaze() {
  double scaleX = (double)glutGet(GLUT_WINDOW_WIDTH) / (GameState.ortho.right - GameState.ortho.left);
  double scaleY = (double)glutGet(GLUT_WINDOW_HEIGHT) / (GameState.ortho.top - GameState.ortho.bottom);

  double offsetX = GameState.player.x - GameState.visibleRadius - GameState.ortho.left;
  double offsetY = GameState.player.y - GameState.visibleRadius - GameState.ortho.bottom;

  double size = GameState.visibleRadius * 2.0 + 1.0;

  glScissor(ceil(offsetX * scaleX),
            ceil(offsetY * scaleY),
            floor(size * scaleX),
            floor(size * scaleY));
}

void renderTiles() {
  glEnable(GL_SCISSOR_TEST);
#ifdef ENABLE_CLIPPING
  clipMaze();

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

  glDisable(GL_SCISSOR_TEST);
}

void addFog() {
  double x = GameState.player.x - GameState.visibleRadius;
  double y = GameState.player.y - GameState.visibleRadius;

  double size = GameState.visibleRadius * 2.0 + 1.0;

  renderFog(x, y, size, size);
}

void renderPlayer() {
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
  renderTiles();
  renderPlayer();

#ifdef ENABLE_CLIPPING
  addFog();
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

void update(int _) {
  UNUSED(_);
  glutPostRedisplay();
  glutTimerFunc(ANIMATION_60_FPS, update, 0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderWorld();

  engineNextFrame();

  glutSwapBuffers();
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  GameState.ortho.top = MAZE_SIZE;
  GameState.ortho.right = MAZE_SIZE;
  GameState.ortho.bottom = 0;
  GameState.ortho.left = 0;

  gluOrtho2D(GameState.ortho.left,
             GameState.ortho.right,
             GameState.ortho.bottom,
             GameState.ortho.top);

  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(800, 800);
  glutCreateWindow("光よ、あれ！");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboardHandler);
  init();
  update(0);
  glutMainLoop();

  return EXIT_SUCCESS;
}
