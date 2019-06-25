#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#define ANIMATION_60_FPS      (1000.0 / 60.0)

#define ANIMATION_ONCE        1
#define ANIMATION_INFINITY    UINT16_MAX

typedef enum AnimationCleanFlag {
  ANIMATION_CLEAN_NONE   = 0,
  ANIMATION_CLEAN_TARGET = 1 << 0,
  ANIMATION_CLEAN_FROM   = 1 << 1,
  ANIMATION_CLEAN_DELTA  = 1 << 2,
} AnimationCleanFlag;

typedef struct Animation {
  void* target;
  void* from;
  void* delta;

  int currentFrame;
  int frameCount;

  int elapsed;
  int interval;

  int nth;
  int repeat;

  AnimationCleanFlag cleanFlag;

  void (*render)(struct Animation*);
  void (*update)(struct Animation*);
  void (*complete)(struct Animation*);
} Animation;

Animation* createAnimation(int frames,
                           double duration,
                           int repeat);
Animation* createAnimation60FPS(double duration, int repeat);

bool cancelAnimation(Animation* animation);

void engineNextFrame(void);
void engineRender(void);

#endif
