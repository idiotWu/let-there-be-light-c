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
 *  - renders one more frame per sencond.
 *
 * 3 frames non-repeat: (3 FPS)
 * [-------|-------]
 * 1       2       3
 *
 * 3 frames * 2 repeats: (3+1 FPS)
 * [----|----|----][----|----|----]
 * 1    2    3   1(4)  2    3   1(4)
 */
Animation* createAnimation(uint16_t frameCount,
                           double duration,
                           uint16_t repeat) {
  assert(frameCount > 0);
  assert(repeat > 0);

  Animation* animation = malloc(sizeof(Animation));

  resetFrame(animation);

  animation->frameCount = frameCount;
  animation->framesPerSecond = repeat == 1 ? frameCount : frameCount + 1;

  animation->interval = round(duration / ANIMATION_60_FPS / animation->framesPerSecond);

  animation->nth = 1;
  animation->repeat = repeat;

  animation->from = NULL;
  animation->to = NULL;
  animation->render = NULL;
  animation->complete = NULL;

  TimelineNode* node = (TimelineNode*)createNode();
  node->data = animation;

  listAppend((List*)&gameTL, (Node*)node);

  return animation;
}

Animation* createAnimation60FPS(double duration, uint16_t repeat) {
  return createAnimation(round(duration / ANIMATION_60_FPS), duration, repeat);
}

bool cancelAnimation(Animation* animation) {
  TimelineNode* node = gameTL.head;

  while (node) {
    if (node->data == animation) {
      listDelete((List*)&gameTL, (Node*)node);
      return true;
    }

    node = node->next;
  }

  return false;
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

    animation->render(animation);

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

    // currentFrame < framesPerSecond
    //  -> running
    if (animation->currentFrame < animation->framesPerSecond) {
      continue;
    }

    // currentFrame == framesPerSecond
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
      // render the last frame
      animation->render(animation);

      if (animation->complete) {
        animation->complete(animation);
      }

      free(animation->from);
      free(animation->to);
      listDelete((List*)&gameTL, (Node*)node);
    }
  } while ((node = next) != NULL);
}
