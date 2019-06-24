#include <stdlib.h>
#include "glut.h"

#include "engine.h"
#include "expr-fx.h"
#include "floodfill.h"
#include "texture.h"
#include "tile.h"
#include "config.h"
#include "state.h"
#include "list.h"
#include "util.h"

#define FX_EXPLODE_DURATION       300
#define FX_EXPLODE_SCALE          1.2
#define FX_FLOOD_DURADION         600
#define FX_FLOOD_INFLEXION_POINT  0.1

// ============ Explode Begin ============ //

static void fxExplodeRender(Animation* animation) {
  int* row = animation->target;
  vec2i* start = animation->from;

  double offset = (1.0 - FX_EXPLODE_SCALE) / 2.0;

  renderSprite(FX_SPRITES, *row,
               animation->currentFrame - 1,
               start->x + offset, start->y + offset, // centering
               FX_EXPLODE_SCALE, FX_EXPLODE_SCALE);
}

void fxExplodeGen(int spriteRow, int x, int y) {
  Animation* animation = createAnimation(FX_SPRITES->cols, FX_EXPLODE_DURATION, 1);

  vec2i* start = malloc(sizeof(vec2i));
  start->x = x;
  start->y = y;

  int* row = malloc(sizeof(int));
  *row = spriteRow;

  animation->target = row;
  animation->from = start;
  animation->cleanFlag = ANIMATION_CLEAN_TARGET | ANIMATION_CLEAN_FROM;

  animation->render = fxExplodeRender;
}

// ============ Explode End ============ //

// ============ Flood Begin ============ //

typedef struct FxFloodRecord {
  FloodState* state;
  FrontierList* frontiers;
} FxFloodRecord;

static void fxFloodRender(Animation* animation);
static void fxFloodUpdate(Animation* animation);
static void fxFloodFinish(Animation* animation);

static void clearSpoiledTile(int x, int y) {
  Tile* tile = &GameState.maze[y][x];

  if (*tile & TILE_SPOILED) {
    clearBit(tile, TILE_SPOILED);
    fxExplodeGen(FX_SMOKE_ROW, x + 0.5, y + 0.5);
  }
}

static void fxFloodNext(FloodState* state) {
  if (state->finished) {
    floodDestory(state);
    return;
  }

  floodForward(state);

  FxFloodRecord* record = malloc(sizeof(FxFloodRecord));

  record->state = state;
  record->frontiers = listClone(state->frontiers, sizeof(Frontier));

  Animation* animation = createAnimation60FPS(FX_FLOOD_DURADION, 1);
  animation->target = record;
  animation->render = fxFloodRender;
  animation->update = fxFloodUpdate;
  animation->complete = fxFloodFinish;
}

static void renderFrontiers(FxFloodRecord* record, double scale, double alpha) {
  ListIterator it = createListIterator(record->frontiers);

  // enable alpha blend
  setTexParam(GL_MODULATE);
  glColor4d(alpha, alpha, alpha, alpha);
  
  while (!it.done) {
    FrontierNode* node = it.next(&it);
    int x = node->data->x;
    int y = node->data->y;

    double ox = x + 0.5;
    double oy = y + 0.5;

    clearSpoiledTile(x, y);

    int row, col;

    switch (GameState.maze[y][x]) {
      case TILE_COIN:
        row = COIN_ROW;
        col = COIN_COL;
        break;

      case TILE_KERNEL:
        row = FIRE_ROW;
        col = FIRE_COL;
        break;

      default:
        row = COIN_DARK_ROW;
        col = COIN_DARK_COL;
        break;
    }

    glPushMatrix();
      glTranslated(ox, oy, 0.0);
      glScaled(scale, scale, 1.0);
      glTranslated(-ox, -oy, 0.0);
      renderSprite(MISC_SPRITES, row, col, x, y, 1, 1);
    glPopMatrix();
  }

  restoreDefaultTexParam();
}

static void fxFloodUpdate(Animation* animation) {
  FxFloodRecord* record = animation->target;
  uint16_t throttle = FX_FLOOD_INFLEXION_POINT * animation->frameCount;

  if (animation->currentFrame >= throttle) {
    fxFloodNext(record->state);
    animation->update = NULL; // remove callback
  }
}

static void fxFloodRender(Animation* animation) {
  FxFloodRecord* record = animation->target;
  uint16_t throttle = FX_FLOOD_INFLEXION_POINT * animation->frameCount;

  double percent;

  if (animation->currentFrame < throttle) {
    percent =(double)animation->currentFrame / throttle;
  } else {
    percent = 1.0 - (double)(animation->currentFrame - throttle) / (animation->frameCount - throttle);
  }

  renderFrontiers(record, percent, percent);
}

static void fxFloodFinish(Animation* animation) {
  FxFloodRecord* record = animation->target;
  listDestory(record->frontiers);
}

void fxFloodGen(int x, int y) {
  FloodState* state = floodGenerate(GameState.maze, x, y);

  fxExplodeGen(FX_EXPLODE_ROW, x, y);

  fxFloodNext(state);

  clearSpoiledTile(x, y);
}

// ============ Flood End ============ //
