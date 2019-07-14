#ifndef SCENE_H
#define SCENE_H

typedef struct Scene {
  void (*init)(void);
  void (*update)(void);
  void (*render)(void);
  void (*destroy)(void);
} Scene;

void switchScene(Scene* nextScene);

#endif
