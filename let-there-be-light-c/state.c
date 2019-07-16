#include <stdbool.h>

#include "config.h"

#include "state.h"
#include "util/list.h"
#include "maze/direction.h"
#include "scene/scene.h"
#include "scene/title/title.h"
#include "scene/game-over/game-over.h"

State GameState;

void initGameState(void) {
  GameState.level.major = 1;
  GameState.level.minor = 1;
  GameState.remainItem = 0;

  GameState.openTiles = NULL;
  GameState.pathLength = 0;

  GameState.paused = false;

  GameState.visibleRadius = 0.0;
  GameState.lastVisibleRadius = 0.0;
  GameState.player.idle = true;
  GameState.player.spoiled = false;
  GameState.player.spriteState = 0;
  
  GameState.enemies = createList();

  switchScene(titleScene);
}
