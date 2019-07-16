#include <stdlib.h>

#include "state.h"

#include "scene/scene.h"

void switchScene(Scene* nextScene) {
  if (nextScene == NULL) {
    return;
  }
  
  if (GameState.scene != NULL) {
    GameState.scene->destroy();
  }

  nextScene->init();

  GameState.scene = nextScene;
}
