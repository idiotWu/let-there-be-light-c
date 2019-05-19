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
void resetFrame(Animation* animation) {
  animation->currentFrame = 0;
  animation->elapsed = 0;
}

Animation* createAnimation(uint16_t totalFrames,
                           double duration,
                           uint16_t repeat) {
  assert(totalFrames > 0);
  assert(repeat > 0);
  
  Animation* animation = malloc(sizeof(Animation));
  
  resetFrame(animation);
  
  animation->totalFrames = totalFrames;
  
  animation->interval = round(duration / ANIMATION_60_FPS / totalFrames);
  
  animation->nth = 1;
  animation->repeat = repeat;
  
  animation->shape = NULL;
  animation->render = NULL;
  animation->complete = NULL;
  
  TimelineNode* node = (TimelineNode*)createNode();
  node->data = animation;
  
  listAppend((List *)&gameTL, (Node *)node);
  
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

/**
 * Right-continuous animation (repeat > 1):
 *  - jumps to 0 frame when the animation ends;
 *  - discards the final frame.
 *
 * 2FPS non-repeat:
 * [--------------|--------------]
 * 0              1              2[final]
 *
 * 2FPS * 2 repeats:
 * [-------|-------|-------|-------]
 * 0       1      0(2)     1      0(2)[discard]
 */
void engineNext(void) {
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
    
    // currentFrame == lastFrame
    //   -> repeat or finish
    if (animation->currentFrame == animation->totalFrames) {
      // repeat animation (jump to 0 frame)
      if (animation->repeat == ANIMATION_INFINITY ||
          animation->nth < animation->repeat) {
        resetFrame(animation);
        animation->nth++;
        continue;
      }
      
      // finished
      // final rendering (100% completed) for non-repeat mode
      if (animation->repeat == 1) {
        animation->render(animation);
      }
      
      if (animation->complete) {
        animation->complete(animation);
      }
      
      free(animation->shape);
      listDelete((List*)&gameTL, (Node*)node);
    }
  } while ((node = next) != NULL);
}
