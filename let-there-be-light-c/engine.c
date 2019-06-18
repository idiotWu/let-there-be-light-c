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

static Timeline gameTL;

// reset state to the beginning of animaiton
static void resetFrame(Animation* animation) {
  animation->currentFrame = 1;
  animation->elapsed = 0;
}

/**
 * Right-continuous animation (repeat > 1):
 *  - jumps to the first frame when the animation ends;
 *  - renders one more frame per period.
 *
 * 3 frames non-repeat: (3 frames/period)
 * [-------|-------]
 * 1       2       3
 *
 * 3 frames * 2 repeats: (3+1 frames/period)
 * [----|----|----][----|----|----]
 * 1    2    3   1(4)  2    3   1(4)
 */
Animation* createAnimation(uint16_t frameCount,
                           double duration,
                           uint16_t repeat) {
  assert(frameCount > 0);
  assert(repeat > 0);

  Animation* animation = calloc(1, sizeof(Animation));

  resetFrame(animation);

  animation->frameCount = frameCount;
  animation->framesPerPeriod = repeat == 1 ? frameCount : frameCount + 1;

  animation->interval = ceil(duration / ANIMATION_60_FPS / animation->framesPerPeriod);

  animation->nth = 1;
  animation->repeat = repeat;
  animation->deleteAfterRender = false;

  animation->from = NULL;
  animation->to = NULL;
  animation->render = NULL;
  animation->update = NULL;
  animation->complete = NULL;

  TimelineNode* node = (TimelineNode*)createNode();
  node->data = animation;

  listAppend((List*)&gameTL, (Node*)node);

  return animation;
}

Animation* createAnimation60FPS(double duration, uint16_t repeat) {
  return createAnimation(round(duration / ANIMATION_60_FPS), duration, repeat);
}

void cancelAnimation(Animation* animation) {
  animation->deleteAfterRender = true;
}

void engineNextFrame(void) {
  if (!gameTL.count) {
    return;
  }

  TimelineNode* node = gameTL.head;
  TimelineNode* next;

  do {
    // record `next` in case that node is deleted
    next = node->next;

    Animation* animation = node->data;

    // skip update
    if (animation->deleteAfterRender) {
      continue;
    }

    animation->elapsed++;

    // elapsed < interval
    //   -> waiting
    if (animation->elapsed < animation->interval) {
      continue;
    }

    // elapsed == interval
    //   -> next frame
    animation->currentFrame++;
    animation->elapsed = 0;

    // invoke update callback
    if (animation->update) {
      animation->update(animation);
    }

    // currentFrame < framesPerPeriod
    //  -> running
    if (animation->currentFrame < animation->framesPerPeriod) {
      continue;
    }

    // currentFrame == framesPerPeriod
    //  -> repeat or finish

    // repeating mode: jump to the first frame
    if (animation->repeat > 1) {
      resetFrame(animation);
    }

    // should repeat?
    if (animation->repeat == ANIMATION_INFINITY ||
        animation->nth < animation->repeat) {
      animation->nth++;
    } else {
      // finished
      animation->deleteAfterRender = true;
    }
  } while ((node = next) != NULL);
}

void engineRender(void) {
  TimelineNode* node = gameTL.head;
  TimelineNode* next;

  while (node) {
    // record `next` in case that node is deleted
    next = node->next;

    Animation* animation = node->data;

    if (animation->render) {
      animation->render(animation);
    }

    if (animation->deleteAfterRender) {
      if (animation->complete) {
        animation->complete(animation);
      }

      free(animation->from);
      free(animation->to);
      listDelete((List*)&gameTL, (Node*)node);
    }

    node = next;
  }
}
