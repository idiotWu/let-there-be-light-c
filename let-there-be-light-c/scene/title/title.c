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
static void destroyTitle(void);

static Scene title = {
  .init = initTitle,
  .update = noop,
  .render = noop,
  .destroy = destroyTitle,
};

Scene* titleScene = &title;

static const char* msg = "PRESS ANY KEY";

static Animation* blink = NULL;

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

static void renderTitle(Animation *animation) {
  double percent = (double)animation->currentFrame / animation->frameCount;
  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  size_t length = strlen(msg);

  setTexParam(GL_MODULATE);
  glColor4d(1.0, 0.5, 0.0, animation->nth % 2 ? percent : 1.0 - percent);
  renderText(msg, (width - length * FONT_SIZE) / 2.0, (height - FONT_SIZE) / 2.0, FONT_SIZE);
  restoreDefaultTexParam();
}

static void initTitle(void) {
  blink = createAnimation60FPS(1000, ANIMATION_INFINITY);
  blink->render = renderTitle;

  glutKeyboardFunc(keyboardHandler);
  glutSpecialFunc(specialKeyHandler);
}

static void destroyTitle(void) {
  cancelAnimation(blink);
  blink = NULL;

  glutKeyboardFunc(NULL);
  glutSpecialFunc(NULL);
}
