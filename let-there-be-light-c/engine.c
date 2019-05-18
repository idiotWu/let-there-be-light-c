#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

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

Animation* createAnimation(uint8_t totalFrames,
                           double duration,
                           uint8_t repeat) {
  assert(totalFrames > 0);
  assert(repeat > 0);
  
  Animation* animation = malloc(sizeof(Animation));
  
  resetFrame(animation);
  
  animation->totalFrames = totalFrames;
  
  animation->interval = duration / ANIMATION_60_FPS / totalFrames;
  
  animation->nth = 1;
  animation->repeat = repeat;
  
  animation->render = NULL;
  animation->complete = NULL;
  
  TimelineNode* node = (TimelineNode*)createNode();
  node->data = animation;
  
  listAppend((List *)&gameTL, (Node *)node);
  
  return animation;
}

Animation* createAnimation60FPS(double duration, uint8_t repeat) {
  return createAnimation(duration / ANIMATION_60_FPS, duration, repeat);
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
 * Right-continuous animation:
 * jumps to 0 frame when the animation ends.
 *
 * 2FPS non-repeat:
 * [--------------|--------------]
 * 0              1              2(final)
 *
 * 2FPS * 2 repeats:
 * [-------|-------|-------|-------]
 * 0       1      0(2)     1      0(2)
 */
void engineRefresh(void) {
  TimelineNode* node = gameTL.head;
  
  if (!node) {
    return;
  }
  
  do {
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
      
      listDelete((List*)&gameTL, (Node*)node);
    }
  } while ((node = node->next) != NULL);
}
