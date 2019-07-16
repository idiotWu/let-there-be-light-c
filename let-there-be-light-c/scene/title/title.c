#include <string.h>
#include <stdio.h>

#include "glut.h"

#include "title.h"

#include "state.h"

#include "scene/scene.h"
#include "render/transition.h"
#include "util/util.h"
#include "render/texture.h"

#define FONT_SIZE 1.2

static void initTitle(void);
static void renderTitle(void);
static void destroyTitle(void);

static Scene title = {
  .init = initTitle,
  .update = noop,
  .render = renderTitle,
  .destroy = destroyTitle,
};

Scene* titleScene = &title;

static const char* caption = "PRESS ANY KEY";

static double alphaDelta = 0.02;
static double captionAlpha = 0.0;

static void startGame(void) {
  destroyTitle();
  levelTransition();
}

static void keyboardHandler(unsigned char key, int x, int y) {
  UNUSED(key); UNUSED(x); UNUSED(y);
  startGame();
}

static void specialKeyHandler(int key, int x, int y) {
  UNUSED(key); UNUSED(x); UNUSED(y);
  startGame();
}

static void renderTitle(void) {
  double width = GameState.ortho.width;
  double height = GameState.ortho.height;
  size_t length = strlen(caption);

  captionAlpha += alphaDelta;

  if (captionAlpha > 1.0) {
    captionAlpha = 1.0;
    alphaDelta *= -1;
  } else if (captionAlpha < 0.0) {
    captionAlpha = 0.0;
    alphaDelta *= -1;
  }

  setTexParam(GL_MODULATE);
  glColor4d(1.0, 0.5, 0.0, captionAlpha);
  renderText(caption, (width - length * FONT_SIZE) / 2.0, (height - FONT_SIZE) / 2.0, FONT_SIZE);
  restoreDefaultTexParam();
}

static void initTitle(void) {
  GameState.level.major = 1;
  GameState.level.minor = 1;
  resetGameState();

  captionAlpha = 0.0;
  alphaDelta = 0.02;

  glutKeyboardFunc(keyboardHandler);
  glutSpecialFunc(specialKeyHandler);
}

static void destroyTitle(void) {
  glutKeyboardFunc(NULL);
  glutSpecialFunc(NULL);
}
