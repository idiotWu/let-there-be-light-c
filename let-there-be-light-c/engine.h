#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#define ANIMATION_60_FPS      (1000.0 / 60.0)

#define ANIMATION_ONCE        1
#define ANIMATION_INFINITY    UINT16_MAX

typedef struct Animation {
  void* from;
  void* to;

  uint16_t currentFrame;
  uint16_t frameCount;

  uint16_t elapsed;
  uint16_t interval;

  uint16_t nth;
  uint16_t repeat;

  bool deleteInNextIteration;

  void (*render)(struct Animation*);
  void (*update)(struct Animation*);
  void (*finish)(struct Animation*);
} Animation;

Animation* createAnimation(uint16_t frames,
                           double duration,
                           uint16_t repeat);
Animation* createAnimation60FPS(double duration, uint16_t repeat);

void cancelAnimation(Animation* animation);

void engineNextFrame(void);
void engineRender(void);

#endif
