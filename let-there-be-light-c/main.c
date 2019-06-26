#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "glut.h"

#include "config.h"
#include "util/util.h"
#include "maze/tile.h"
#include "maze/maze.h"
#include "game/state.h"
#include "game/game.h"
#include "render/fx.h"
#include "render/render.h"
#include "render/engine.h"
#include "render/texture.h"
#include "maze/direction.h"

#define GAME_TITLE "Let There Be Light"
#define WIN_WIDTH  800
#define WIN_HEIGHT 800
//#define CENTERING_WINDOW

static int frameCount = 0;

// map the given key code to a direction
Direction keyToDirection(int key) {
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

void keydownHandler(int key, int x, int y) {
  UNUSED(x); UNUSED(y);
  setBits(GameState.keyPressed, keyToDirection(key));
}

void keyupHandler(int key, int x, int y) {
  UNUSED(x); UNUSED(y);
  clearBits(GameState.keyPressed, keyToDirection(key));
}

void init(void) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  initGameState();
  initTextures();
  initGame();
  nextLevel(0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  switch (GameState.currentScene) {
    case SCENE_STAGE_TITLE:
      renderStageTitle();
      break;

    case SCENE_GAME_STAGE:
      renderWorld();
      break;

    default:
      break;
  }

  engineRender();

  glutSwapBuffers();
}

void updateTitle(int fps) {
  char tmp[128];

  sprintf(tmp, "%s: %2d FPS @ %d x %d",
          GAME_TITLE,
          fps,
          glutGet(GLUT_WINDOW_WIDTH),
          glutGet(GLUT_WINDOW_HEIGHT));

  glutSetWindowTitle(tmp);
}

void update(int timestamp) {
  // count FPS
  frameCount++;

  int now = glutGet(GLUT_ELAPSED_TIME);

  if (now - timestamp >= 1000) {
    updateTitle(frameCount);
    timestamp = now;
    frameCount = 0;
  }

  glutTimerFunc(ANIMATION_60_FPS, update, timestamp);

  updateGame();
  engineNextFrame();

#ifdef __APPLE__
  // `glutPostRedisplay()` blocks the next timer on mac
  display();
#else
  glutPostRedisplay();
#endif
}

// resize viewport to center of current window
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

  glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);

#ifdef CENTERING_WINDOW
  int winX = (glutGet(GLUT_SCREEN_WIDTH) - WIN_WIDTH) / 2;
  int winY = (glutGet(GLUT_SCREEN_HEIGHT) - WIN_HEIGHT) / 2;
  glutInitWindowPosition(winX, winY);
#endif

  glutCreateWindow(GAME_TITLE);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  // keyboard handlers
  glutSpecialFunc(keydownHandler);
  glutSpecialUpFunc(keyupHandler);

  init();
  update(glutGet(GLUT_ELAPSED_TIME));
  updateTitle(1000 / ANIMATION_60_FPS);

  glutMainLoop();

  return EXIT_SUCCESS;
}
