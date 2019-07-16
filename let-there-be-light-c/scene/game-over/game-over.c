#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "glut.h"

#include "game-over.h"

#include "config.h"
#include "state.h"

#include "scene/scene.h"
#include "render/transition.h"
#include "scene/title/title.h"
#include "scene/level-title/level-title.h"
#include "scene/game/game.h"
#include "util/util.h"
#include "render/texture.h"
#include "render/engine.h"
#include "render/fx.h"

#define CAPTION_FONT_SIZE           2.0
#define HINT_FONT_SIZE              0.75
#define CAPTION_ANIMATION_DURATION  150

#define RANDOM_CHAR           ((unsigned char)randomInt('A', 'Z'))

static void initGameOver(void);
static void renderGameOver(void);
static void destroyGameOver(void);

static Scene gameOver = {
  .init = initGameOver,
  .update = noop,
  .render = renderGameOver,
  .destroy = destroyGameOver,
};

Scene* gameOverScene = &gameOver;

static const char* caption = "GAME OVER";

static const char* hint = "[R]ETRY   [T]ITLE";

static double hintAlpha = 0.0;
static bool captionAnimationFinished = false;

// ============ Render Text ============ //

static void showMessage(const char* str) {
  size_t length = strlen(str);
  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  char* tmp = malloc(sizeof(char) * (length + 1));
  strcpy(tmp, str);

  int randomIdx = randomInt(0, (int)length - 1);

  if (tmp[randomIdx] == ' ') {
    randomIdx--;
  }

  // replace one letter randomly
  tmp[randomIdx] = RANDOM_CHAR;

  setTexParam(GL_MODULATE);
  glColor3d(0.6, 0.2, 0.2);
  renderText(tmp, (width - length * CAPTION_FONT_SIZE) / 2.0, (height - CAPTION_FONT_SIZE) / 2.0, CAPTION_FONT_SIZE);
  restoreDefaultTexParam();

  free(tmp);
}

static void showHint(void) {
  size_t length = strlen(hint);
  double width = GameState.ortho.width;

  setTexParam(GL_MODULATE);
  glColor4d(1.0, 1.0, 1.0, hintAlpha);
  renderText(hint, (width - length * HINT_FONT_SIZE) / 2.0, 1.0, HINT_FONT_SIZE);
  restoreDefaultTexParam();
}

// ============ Hint Fading In ============ //

static void keyboardHandler(unsigned char key, int x, int y) {
  UNUSED(x); UNUSED(y);

  if (key == 'R' || key == 'r') {
    resetGameState();
    levelTransition();
    destroyGameOver();
    return;
  }

  if (key == 'T' || key == 't') {
    resetGameState();
    fxFadeOut(1000, titleScene);
    destroyGameOver();
    return;
  }
}

static void hintFadeInRender(Animation* animation) {
  hintAlpha = (double)animation->currentFrame / animation->frameCount;
}

static void hintFadeIn(void) {
  Animation* animation = createAnimation60FPS(300, 1);
  animation->update = hintFadeInRender;

  glutKeyboardFunc(keyboardHandler);
}

// ============ Caption Animation ============ //

static void captionComplete(Animation* _) {
  UNUSED(_);

  hintFadeIn();
  captionAnimationFinished = true;
}

static void captionRender(Animation* animation) {
  size_t n = animation->nth;

  char* str = malloc(sizeof(char) * (n + 1));
  strncpy(str, caption, n - 1);
  str[n - 1] = caption[n - 1] == ' ' ? ' ' : RANDOM_CHAR;
  str[n] = '\0';

  showMessage(str);
  free(str);
}

// ============ Misc ============ //

static void initGameOver(void) {
  hintAlpha = 0.0;
  captionAnimationFinished = false;

  Animation* animation = createAnimation60FPS(CAPTION_ANIMATION_DURATION, (int)strlen(caption));

  animation->render = captionRender;
  animation->complete = captionComplete;
}

static void renderGameOver(void) {
  if (!captionAnimationFinished) {
    return;
  }

  showMessage(caption);
  showHint();
}

static void destroyGameOver(void) {
  glutKeyboardFunc(NULL);
}

