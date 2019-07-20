/**
 * @file
 * @brief ゲームの難易度の計算
 */
#include "scene/game/difficulty.h"

#include "config.h"
#include "state.h"

#include "util/util.h"

/**
 * @brief 0を始まりとするメージャーレベル（ワールド）を計算する
 *
 * @return int メージャーレベル
 */
static int getMajorLevel(void) {
  return GameState.level.major - 1;
}

/**
 * @brief 0を始まりとするマイナーレベル（ステージ）を計算する
 *
 * @return int マイナーレベル
 */
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
  // easter eggs!
  if (GameState.unlockLimits &&
      GameState.level.major == 1 && GameState.level.minor == 1) {
    return MAX_VISIBLE_RADIUS;
  }

  double baseRadius = max(BASE_VISIBLE_RADIUS_MIN, BASE_VISIBLE_RADIUS - BASE_VISIBLE_RADIUS_REDUCING_RATE * getMajorLevel());

  double limit = GameState.unlockLimits ? 0.75 : 0.5;

  double visibleRadius = baseRadius + GameState.lastVisibleRadius * limit;

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
