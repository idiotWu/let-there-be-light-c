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

#define INFLEXION_POINT  0.1
#define FX_DURATION      600

typedef struct FxRecord {
  FloodState* state;
  FrontierList* frontiers;
} FxRecord;

static void fxRender(Animation* animation);
static void fxUpdate(Animation* animation);
static void fxFinish(Animation* animation);

static void fxNext(FloodState* state) {
  if (state->finished) {
    floodDestory(state);
    return;
  }

  floodForward(state);

  FxRecord* record = malloc(sizeof(FxRecord));

  record->state = state;
  record->frontiers = listClone(state->frontiers, sizeof(Frontier));

  Animation* animation = createAnimation60FPS(FX_DURATION, 1);
  animation->from = record;
  animation->render = fxRender;
  animation->update = fxUpdate;
  animation->complete = fxFinish;
}

static void renderRecord(FxRecord* record, double scale, double alpha) {
  ListIterator it = createListIterator(record->frontiers);

  setTexParam(GL_MODULATE);
  glColor4d(alpha, alpha, alpha, alpha);
  
  while (!it.done) {
    FrontierNode* node = it.next(&it);
    int x = node->data->x;
    int y = node->data->y;

    double ox = x + 0.5;
    double oy = y + 0.5;

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

static void fxUpdate(Animation* animation) {
  FxRecord* record = animation->from;
  uint16_t throttle = INFLEXION_POINT * animation->frameCount;

  if (animation->currentFrame >= throttle) {
    fxNext(record->state);
    animation->update = NULL; // remove callback
  }
}

static void fxRender(Animation* animation) {
  FxRecord* record = animation->from;
  uint16_t throttle = INFLEXION_POINT * animation->frameCount;

  double percent;

  if (animation->currentFrame < throttle) {
    percent =(double)animation->currentFrame / throttle;
  } else {
    percent = 1.0 - (double)(animation->currentFrame - throttle) / (animation->frameCount - throttle);
  }

  renderRecord(record, percent, percent);
}

static void fxFinish(Animation* animation) {
  FxRecord* record = animation->from;
  listDestory(record->frontiers);
}

static void fxExplode(Animation* animation) {
  Frontier* start = animation->from;

  renderSprite(FX_EXPLODE_SPRITES, 0, animation->currentFrame - 1, start->x - 0.5, start->y - 0.5, 2.0, 2.0);
}

void fxGen(int x, int y) {
  FloodState* state = floodGenerate(GameState.maze, x, y);

  Animation* animation = createAnimation(FX_EXPLODE_SPRITES->cols, 300, 1);

  Frontier* start = malloc(sizeof(Frontier));
  start->x = x;
  start->y = y;

  animation->from = start;
  animation->render = fxExplode;

  fxNext(state);
}
