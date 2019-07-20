/**
 * @file
 * @brief シーンの切り替えるアニメーション
 */
#include <stdlib.h>
#include "glut.h"

#include "state.h"

#include "render/transition.h"

#include "scene/scene.h"
#include "scene/game/game.h"
#include "scene/level-title/level-title.h"
#include "render/engine.h"
#include "render/fx.h"
#include "util/util.h"

/**
 * @internal
 * @brief 一連の切り替えを表すオブジェクト
 */
typedef struct TransitionQueue {
  //! 切り替えアニメーションの持続時間
  double duration;
  //! 目標のシーン
  Scene* target;

  //! 途中のシーンの表示時間
  double rest;
  //! 途中のシーン
  Scene* midway;
} TransitionQueue;

/**
 * @brief 三角形を描画する
 *
 * @param vertexes 3 つの頂点の座標
 */
static void drawTriangle(vec2d vertexes[3]) {
  glBegin(GL_POLYGON);
  for (size_t i = 0; i < 3; i++) {
    glVertex2d(vertexes[i].x, vertexes[i].y);
  }
  glEnd();
}

/**
 * @brief \ref transitionIn アニメーションをレンダリングする
 *
 * @param animation アニメーション
 */
static void transitionInRender(Animation* animation) {
  double percent = (double)animation->currentFrame / animation->frameCount;

  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  vec2d above[3] = {
    { 0.0, height },
    { width * percent, height },
    { 0.0, height * (1.0 - percent) },
  };

  vec2d below[3] = {
    { width, 0.0 },
    { width, height * percent},
    { width * (1.0 - percent), 0.0 },
  };

  glColor3d(1.0, 1.0, 1.0);
  drawTriangle(above);
  drawTriangle(below);
}

/**
 * @brief \ref transitionOut アニメーションをレンダリングする
 *
 * @param animation アニメーション
 */
static void transitionOutRender(Animation* animation) {
  double percent = (double)animation->currentFrame / animation->frameCount;

  double width = MAZE_SIZE;
  double height = MAZE_SIZE + HUD_HEIGHT;

  vec2d above[3] = {
    { 0.0, height },
    { width * (1.0 - percent), height },
    { 0.0, height * percent },
  };

  vec2d below[3] = {
    { width, 0.0 },
    { width, height * (1.0 - percent) },
    { width * percent, 0.0 },
  };

  glColor3d(1.0, 1.0, 1.0);
  drawTriangle(above);
  drawTriangle(below);
}

Animation* transitionOut(double duration) {
  Animation* animation = createAnimation60FPS(duration, 1);

  animation->render = transitionOutRender;

  return animation;
}

Animation* transitionIn(double duration) {
  Animation* animation = createAnimation60FPS(duration, 1);

  animation->render = transitionInRender;

  return animation;
}

// ============ Level Transition ============ //

/**
 * @brief 一連の切り替えが終わった後の処理
 *
 * @details 途中のシーンを一定の時間に表示するため，この関数は遅れて実行される
 *
 * @param _queue 切り替えを表すオブジェクト
 */
static void transitionQueueFinish(void* _queue) {
  TransitionQueue* queue = _queue;

  switchScene(queue->target);
  transitionOut(queue->duration);

  free(queue);
}

/**
 * @brief 途中のシーンに切り替える
 *
 * @param animation 切り替えのアニメーション
 */
static void transitionQueueMidway(Animation* animation) {
  TransitionQueue* queue = animation->target;

  switchScene(queue->midway);
  delay(queue->rest, transitionQueueFinish, queue);
}

void transitionQueue(double duration, Scene* target, double rest, Scene* midway) {
  TransitionQueue* queue = malloc(sizeof(TransitionQueue));
  queue->duration = duration;
  queue->target = target;
  queue->rest = rest;
  queue->midway = midway;

  Animation* in = transitionIn(duration);
  in->target = queue;
  in->complete = transitionQueueMidway;
  in->cleanFlag = ANIMATION_CLEAN_NONE;
}

void levelTransition(void) {
  transitionQueue(LEVEL_TRANSITION_DURATION, gameScene,
                  LEVEL_TRANSITION_REST, levelTitleScene);
}
