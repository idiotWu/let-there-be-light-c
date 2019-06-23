#include <stdbool.h>

#include "state.h"
#include "direction.h"

State GameState = {
  .level = 0,
  .idle = true,
  .cleared = false,
  .keyPressed = DIR_NONE,
  .player = { .state = 0, }
};
