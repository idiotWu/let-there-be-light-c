#include <string.h>
#include <stdio.h>

#include "glut.h"

#include "level-title.h"

#include "state.h"

#include "scene/scene.h"
#include "util/util.h"
#include "engine/texture.h"

static void renderLevelTitle(void);

static Scene levelTitle = {
  .init = noop,
  .update = noop,
  .render = renderLevelTitle,
  .destroy = noop,
};

Scene* levelTitleScene = &levelTitle;

static void renderLevelTitle(void) {
  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  char title[128];
  sprintf(title, "WORLD %d-%d", GameState.level / LEVEL_INTERVAL + 1, GameState.level % LEVEL_INTERVAL + 1);

  size_t length = strlen(title);

  glColor3d(1.0, 1.0, 1.0);
  glRectd(0.0, 0.0, width, height);

  setTexParam(GL_MODULATE);
  glColor3d(0.0, 0.0, 0.0);
  renderText(title, (width - length) / 2.0, (height - 1.0) / 2.0, 1.0);
  restoreDefaultTexParam();
}
