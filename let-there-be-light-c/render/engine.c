/**
 * @file
 * @brief タイムラインに基づいたアニメーションエンジン
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "render/engine.h"
#include "util/list.h"

/**
 * @internal
 * @brief 遅らせている関数および引数に渡すデータを記録するオブジェクト
 */
typedef struct Delay {
  //! コールバック関数
  DelayCallback callback;
  //! `callback` の引数に渡すデータ
  void* data;
} Delay;

//! エンジンのメインタイムライン
static List GameTL;

Animation* createAnimation(int frameCount, double duration, int repeat) {
  assert(frameCount > 0);
  assert(repeat > 0);

  Animation* animation = calloc(1, sizeof(Animation));

  animation->elapsed = 0;

  // animation begins at the first frame
  animation->currentFrame = 1;

  animation->nth = 1;
  animation->repeat = repeat;
  animation->frameCount = frameCount;

  animation->interval = ceil(duration / ANIMATION_REFRESH_INTERVAL / animation->frameCount);

  animation->target = NULL;
  animation->from = NULL;
  animation->delta = NULL;
  animation->render = NULL;
  animation->update = NULL;
  animation->complete = NULL;

  animation->deleteInNextIteration = false;
  animation->cleanFlag = ANIMATION_CLEAN_FROM | ANIMATION_CLEAN_DELTA;

  Node* node = createNode();
  node->data = animation;

  listAppend(&GameTL, node);

  return animation;
}

Animation* createAnimation60FPS(double duration, int repeat) {
  return createAnimation(round(duration / ANIMATION_REFRESH_INTERVAL), duration, repeat);
}

void cancelAnimation(Animation* animation) {
  if (animation == NULL) {
    return;
  }

  animation->deleteInNextIteration = true;
}

static void cleanAnimation(Animation* animation) {
  if (animation->cleanFlag & ANIMATION_CLEAN_TARGET) {
    free(animation->target);
  }

  if (animation->cleanFlag & ANIMATION_CLEAN_FROM) {
    free(animation->from);
  }

  if (animation->cleanFlag & ANIMATION_CLEAN_DELTA) {
    free(animation->delta);
  }
}

void engineNextFrame(void) {
  ListIterator it = createListIterator(&GameTL);

  while (!it.done) {
    Node* node = it.next(&it);

    Animation* animation = node->data;

    // remove marked animation
    if (animation->deleteInNextIteration) {
      cleanAnimation(animation);
      listDelete(&GameTL, node);
      continue;
    }

    // elapsed < interval
    //  -> waiting
    if (++animation->elapsed < animation->interval) {
      continue;
    }

    if (animation->currentFrame < animation->frameCount) {
      // still running:
      //  go to next frame
      animation->elapsed = 0;
      animation->currentFrame++;
    } else if (animation->repeat == ANIMATION_INFINITY ||
               animation->nth < animation->repeat) {
      // repeating:
      //  reset to the beginning of the next period
      animation->nth++;
      animation->elapsed = 0;
      animation->currentFrame = 1;
    } else {
      // finished:
      //  invoke complete callback
      if (animation->complete) {
        animation->complete(animation);
      }

      cleanAnimation(animation);
      listDelete(&GameTL, node);

      continue;
    }

    // invoke update callback
    if (animation->update) {
      animation->update(animation);
    }
  }
}

void engineRender(void) {
  ListIterator it = createListIterator(&GameTL);

  while (!it.done) {
    Node* node = it.next(&it);

    Animation* animation = node->data;

    if (animation->render) {
      animation->render(animation);
    }
  }
}

static void invokeDelay(Animation* animation) {
  Delay* delay = animation->target;

  delay->callback(delay->data);
}

Animation* delay(double duration, DelayCallback callback, void* data) {
  Delay* delay = malloc(sizeof(Delay));

  delay->callback = callback;
  delay->data = data;

  Animation* animation = createAnimation(1, duration, 1);

  animation->target = delay;
  animation->cleanFlag = ANIMATION_CLEAN_TARGET;
  animation->complete = invokeDelay;

  return animation;
}
