
#include "scene/game/difficulty.h"

#include "config.h"
#include "state.h"

#include "util/util.h"

static int getMajorLevel(void) {
  return GameState.level.major - 1;
}

static int getMinorLevel(void) {
  return GameState.level.minor - 1;
}

int getSpawnerCount(void) {
  // min, min+1, min+2, ...
  return MIN_SPAWNER_COUNT + getMajorLevel();
}

int getMinBuilderDistance(void) {
  // 2n, 2n-2, ..., 2
  return 2 * (LEVEL_INTERVAL - getMinorLevel());
}

double getInitialVisibleRadius(void) {
  double baseRadius = max(BASE_VISIBLE_RADIUS_MIN, BASE_VISIBLE_RADIUS - BASE_VISIBLE_RADIUS_REDUCING_RATE * getMajorLevel());

  double visibleRadius = baseRadius + GameState.lastVisibleRadius / 2.0;

  double limit = GameState.unlockLimits ? 1.0 : 0.5;

  return min(visibleRadius, MAX_VISIBLE_RADIUS * limit);
}

double getVisibleRadiusRuducingRate(void) {
  return RADIUS_REDUCING_RATE * (GameState.player.spoiled ? SPOILED_DAMAGE : 1.0);
}

// item desity: (0.5, 1]
double getItemDensity(void) {
  double density = 1.0 - (double)getMinorLevel() / LEVEL_INTERVAL * 0.5;

  return clamp(density, 0.5, 1.0);
}

double getEnemySpawnIntervel(void) {
  double intervel = ENEMY_SPAWN_INTERVEL - ENEMY_SPAWN_INTERVEL_REDUCING_RATE * getMajorLevel();

  return max(intervel, ENEMY_SPAWN_INTERVEL_MIN);
}
