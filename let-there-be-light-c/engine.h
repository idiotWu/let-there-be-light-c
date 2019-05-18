#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#define ANIMATION_60_FPS 16.666666667

#define ANIMATION_ONCE 1
#define ANIMATION_INFINITY UINT8_MAX

typedef struct Animation {
  uint8_t currentFrame;
  uint8_t totalFrames;
  
  uint8_t elapsed;
  uint8_t interval;
  
  uint8_t nth;
  uint8_t repeat;
  
  void (*render)(struct Animation*);
  void (*complete)(struct Animation*);
} Animation;

Animation* createAnimation(uint8_t totalFrames,
                           double duration,
                           uint8_t repeat);
Animation* createAnimation60FPS(double duration, uint8_t repeat);

bool cancelAnimation(Animation* animation);

void engineRefresh(void);

#endif
