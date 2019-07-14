#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "glut.h"

#include "game-over.h"

#include "config.h"
#include "state.h"

#include "scene/scene.h"
#include "util/util.h"
#include "engine/texture.h"
#include "engine/engine.h"

#define FONT_SIZE     2.0
#define DURATION      250
#define START_RADIUS  (MAZE_SIZE)

static void initGameOver(void);
static void renderGameOver(void);

static Scene gameOver = {
  .init = initGameOver,
  .update = noop,
  .render = renderGameOver,
  .destroy = noop,
};

Scene* gameOverScene = &gameOver;

const char* msg = "GAME OVER";
const size_t length = 9;

bool animationFinished = false;

static void showMessage(const char* str) {
  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  char* tmp = malloc(length);
  strcpy(tmp, str);

  int randomIdx = randomInt(0, length - 1);

  if (tmp[randomIdx] == ' ') {
    randomIdx++;
  }

  // replace one letter randomly
  tmp[randomIdx] = (unsigned char)randomInt('!', 'z');

  //  size_t length = strlen(str);

  setTexParam(GL_MODULATE);
  glColor3d(0.6, 0.2, 0.2);
  renderText(tmp, (width - length * FONT_SIZE) / 2.0, (height - FONT_SIZE) / 2.0, FONT_SIZE);
  restoreDefaultTexParam();

  free(tmp);
}

static void renderGameOver(void) {
  if (!animationFinished) {
    return;
  }

  showMessage(msg);
}

static void gameOverAnimationComplete(Animation* _) {
  UNUSED(_);

  animationFinished = true;
}

static void gameOverAnimationRender(Animation* animation) {
  size_t n = animation->nth;

  char* str = malloc(sizeof(char) * (n + 1));

  for (size_t i = 0; i < n - 1; i++) {
    str[i] = msg[i];
  }

  str[n - 1] = msg[n - 1] == ' ' ? ' ' : (unsigned char)randomInt('!', 'z');
  str[n] = '\0';

  showMessage(str);
  free(str);
}

static void initGameOver(void) {
  animationFinished = false;

  Animation* animation = createAnimation60FPS(DURATION, length);

  animation->render = gameOverAnimationRender;
  animation->complete = gameOverAnimationComplete;
}

