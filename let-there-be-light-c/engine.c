/**
 * Timeline based animation engine
 *
 *       t
 * ------+------------------------------------------> timeline
 * +-----|-+
 * |   A | | <- animation{}
 * +-----|-+
 *    +--|------+
 *    |  | A    |
 *    +--|------+
 *  +----|+-----+-----+-----+-----+
 *  |  A ||  A  |  A  |  A  |  A  | <- repeating animation
 *  +----|+-----+-----+-----+-----+
 *       |
 *    render(t)
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "engine.h"
#include "list.h"

defNode(TimelineNode, Animation);
defList(Timeline, TimelineNode);

static Timeline GameTL;

/**
 * Animation Model:
 *  3 frames/period non-repeat:
 *   begin <-------|-------|-------> end
 *             1       2       3
 *
 *  3 frames/period * 2 repeats:
 *    begin <---|---|---><---|---|---> end
 *            1   2   3    1   2   3
 */
Animation* createAnimation(uint16_t frameCount,
                           double duration,
                           uint16_t repeat) {
  assert(frameCount > 0);
  assert(repeat > 0);

  Animation* animation = calloc(1, sizeof(Animation));

  animation->elapsed = 0;

  // animation begins at the first frame
  animation->currentFrame = 1;

  animation->nth = 1;
  animation->repeat = repeat;
  animation->frameCount = frameCount;

  animation->interval = ceil(duration / ANIMATION_60_FPS / animation->frameCount);

  animation->deleteInNextIteration = false;

  animation->from = NULL;
  animation->to = NULL;
  animation->render = NULL;
  animation->update = NULL;
  animation->finish = NULL;

  TimelineNode* node = createNode();
  node->data = animation;

  listAppend(&GameTL, node);

  return animation;
}

Animation* createAnimation60FPS(double duration, uint16_t repeat) {
  return createAnimation(round(duration / ANIMATION_60_FPS), duration, repeat);
}

void cancelAnimation(Animation* animation) {
  animation->deleteInNextIteration = true;
}

void engineNextFrame(void) {
  ListIterator it = createListIterator(&GameTL);

  while (!it.done) {
    TimelineNode* node = it.next(&it);

    Animation* animation = node->data;

    // delete the marked animation
    if (animation->deleteInNextIteration) {
      if (animation->finish) {
        animation->finish(animation);
      }

      free(animation->from);
      free(animation->to);
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
      //  mark the animation as to-be-deleted
      animation->deleteInNextIteration = true;
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
    TimelineNode* node = it.next(&it);

    Animation* animation = node->data;

    if (animation->render) {
      animation->render(animation);
    }
  }
}
