#include <stdlib.h>
#include "glut.h"

#include "engine.h"
#include "expr-fx.h"
#include "floodfill.h"
#include "texture.h"
#include "state.h"
#include "list.h"
#include "util.h"

#define INFLEXION_POINT  0.2
#define FX_DURATION      300

#define RANDOM_COLOR

typedef struct FxRecord {
  FloodState* state;
  FrontierList* frontiers;
} FxRecord;

static void fxRender(Animation* animation);
static void fxUpdate(Animation* animation);
static void fxComplete(Animation* animation);

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
  animation->complete = fxComplete;
}

static void renderRecord(FxRecord* record, double scale, double alpha) {
#ifdef RANDOM_COLOR
  glColor4d(randomBetween(0.25, 1),
            randomBetween(0.25, 1),
            randomBetween(0.25, 1),
            alpha);
#else
  glColor4d(0.94, 0.57, 0.30, alpha);
#endif

  ListIterator it = createListIterator(record->frontiers);

   while (!it.done) {
    FrontierNode* node = it.next(&it);
    double x = node->data->x;
    double y = node->data->y;

    glPushMatrix();
      glTranslated(x + 0.5, y + 0.5, 0.0);
      glScaled(scale, scale, 1.0);
      glTranslated(-(x + 0.5), -(y + 0.5), 0.0);
      glRectd(x, y, x + 1.0, y + 1.0);
    glPopMatrix();
  }
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

static void fxComplete(Animation* animation) {
  FxRecord* record = animation->from;
  listDestory(record->frontiers);
}

static void fxExplode(Animation* animation) {
  Frontier* start = animation->from;

  renderSprite(FX_EXPLODE_SPRITES, 0, animation->currentFrame - 1, start->x - 0.5, start->y - 0.5, 2.0, 2.0);
}

void fxGen(void) {
  FloodState* state = floodGenerate(GameState.map,
                                    GameState.player.x,
                                    GameState.player.y);

  Animation* animation = createAnimation(FX_EXPLODE_SPRITES->cols, 300, 1);

  Frontier* start = malloc(sizeof(Frontier));
  start->x = GameState.player.x;
  start->y = GameState.player.y;

  animation->from = start;
  animation->render = fxExplode;

  fxNext(state);
}
