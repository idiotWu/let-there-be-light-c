#include <stdlib.h>

#include "state.h"

#include "scene.h"

void switchScene(Scene* nextScene) {
  if (GameState.scene != NULL) {
    GameState.scene->destroy();
  }

  nextScene->init();

  GameState.scene = nextScene;
}
