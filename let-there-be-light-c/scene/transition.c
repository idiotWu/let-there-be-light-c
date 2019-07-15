#include <stdlib.h>
#include "glut.h"

#include "transition.h"

#include "state.h"
#include "scene.h"

#include "engine/engine.h"
#include "util/util.h"

#define LEVEL_TRANSITION_DURATION   300

typedef struct TransitionQueue {
  double duration;
  Scene* target;

  double rest;
  Scene* midway;
} TransitionQueue;

static void drawTriangle(vec2d vertexes[3]) {
  glBegin(GL_POLYGON);
  for (size_t i = 0; i < 3; i++) {
    glVertex2d(vertexes[i].x, vertexes[i].y);
  }
  glEnd();
}

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

static void transitionQueueFinish(void* _queue) {
  TransitionQueue* queue = _queue;

  switchScene(queue->target);
  transitionOut(queue->duration);

  free(queue);
}

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
