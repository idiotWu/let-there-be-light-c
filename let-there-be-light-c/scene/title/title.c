#include <string.h>
#include <stdio.h>

#include "glut.h"

#include "state.h"
#include "config.h"

#include "scene/title/title.h"

#include "scene/scene.h"
#include "render/transition.h"
#include "util/util.h"
#include "render/texture.h"
#include "render/helper.h"
#include "render/fx.h"

#define FONT_SIZE     1.2
#define FOG_RADIUS    3.0
#define SPRITE_SIZE   1.0
#define EXPLODE_SIZE  3.0

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

static vec2d sprite = { 0.0, 0.0 };
static vec2d mouse = { -100.0, -100.0 };

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

static vec2d xy2uv(int x, int y) {
  ClientRect* vp = &GameState.viewport;
  ClientRect* ortho = &GameState.ortho;
  vec2d uv;

  uv.x = (x - vp->left) * (ortho->width / vp->width);
  uv.y = (vp->top - y) * (ortho->height / vp->height);

  return uv;
}

static void mouseDownHandler(int button, int state, int x, int y) {
  if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) {
    return;
  }

  vec2d uv = xy2uv(x, y);

  double d = distance(uv.x, uv.y, sprite.x + SPRITE_SIZE / 2.0, sprite.y + SPRITE_SIZE / 2.0);

  if (d <= SPRITE_SIZE / 2.0) {
    GameState.unlockLimits = true;

    fxExplode(FX_EXPLODE_ROW, uv.x, uv.y, EXPLODE_SIZE);

    // remove listeners
    glutMouseFunc(NULL);
    glutMotionFunc(NULL);
    glutPassiveMotionFunc(NULL);
  }
}

static void mouseMoveHandler(int x, int y) {
  vec2d uv = xy2uv(x, y);

  mouse.x = uv.x;
  mouse.y = uv.y;
}

static void renderEasterEgg(void) {
  double x = mouse.x - FOG_RADIUS;
  double y = mouse.y - FOG_RADIUS;
  double width = FOG_RADIUS * 2.0;
  double height = FOG_RADIUS * 2.0;

  glEnable(GL_STENCIL_TEST);
  clipRect(x, y, width, height);

  renderSprite(PLAYER_SPRITES, CHARACTER_FRONT_ROW, 0,
               sprite.x, sprite.y, SPRITE_SIZE, SPRITE_SIZE);
  glDisable(GL_STENCIL_TEST);

  renderFog(x, y, width, height);
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

  if (!GameState.unlockLimits) {
    renderEasterEgg();
  }

  setTexParam(GL_MODULATE);

  if (GameState.unlockLimits) {
    glColor4d(1.0, 0.5, 0.0, captionAlpha);
  } else {
    glColor4d(1.0, 1.0, 1.0, captionAlpha);
  }

  renderText(caption, (width - length * FONT_SIZE) / 2.0, (height - FONT_SIZE) / 2.0, FONT_SIZE);
  restoreDefaultTexParam();
}

static void initTitle(void) {
  GameState.unlockLimits = false;
  GameState.level.major = 1;
  GameState.level.minor = 1;

  sprite.x = randomBetween(GameState.ortho.left, GameState.ortho.right - 1);
  sprite.y = randomBetween(GameState.ortho.bottom, GameState.ortho.top - 1);

  mouse.x = mouse.y = -100.0;

  captionAlpha = 0.0;
  alphaDelta = 0.02;

  // keyboard handlers
  glutKeyboardFunc(keyboardHandler);
  glutSpecialFunc(specialKeyHandler);

  // mouse handlers
  glutMouseFunc(mouseDownHandler);
  glutMotionFunc(mouseMoveHandler);
  glutPassiveMotionFunc(mouseMoveHandler);
}

static void destroyTitle(void) {
  glutKeyboardFunc(NULL);
  glutSpecialFunc(NULL);
}
