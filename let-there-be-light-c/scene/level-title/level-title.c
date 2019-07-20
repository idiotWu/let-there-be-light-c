/**
 * @file
 * @brief ステージのタイトルシーンのコントローラー
 */
#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>

#include "glut.h"

#include "state.h"

#include "scene/level-title/level-title.h"

#include "scene/scene.h"
#include "util/util.h"
#include "render/texture.h"

//! タイトルのフォントサイズ
#define FONT_SIZE 1.2

//! ステージのタイトルをレンダリングする
static void renderLevelTitle(void);

//! ステージのタイトルシーン
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
  sprintf(title, "WORLD %d-%d", GameState.level.major, GameState.level.minor);

  size_t length = strlen(title);

  glColor3d(1.0, 1.0, 1.0);
  glRectd(0.0, 0.0, width, height);

  setTexParam(GL_MODULATE);
  glColor3d(0.0, 0.0, 0.0);
  renderText(title, (width - length * FONT_SIZE) / 2.0, (height - FONT_SIZE) / 2.0, FONT_SIZE);
  restoreDefaultTexParam();
}
