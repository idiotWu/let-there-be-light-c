#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#define ANIMATION_60_FPS      (1000.0 / 60.0)

#define ANIMATION_ONCE        1
#define ANIMATION_INFINITY    UINT16_MAX

typedef struct Animation {
  uint16_t currentFrame;
  uint16_t totalFrames;
  
  uint16_t elapsed;
  uint16_t interval;
  
  uint16_t nth;
  uint16_t repeat;
  
  void (*render)(struct Animation*);
  void (*complete)(struct Animation*);
} Animation;

Animation* createAnimation(uint16_t totalFrames,
                           double duration,
                           uint16_t repeat);
Animation* createAnimation60FPS(double duration, uint16_t repeat);

bool cancelAnimation(Animation* animation);

void engineNext(void);

#endif