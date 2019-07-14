#include <stdlib.h>
#include "glut.h"

#include "config.h"
#include "state.h"

#include "fx.h"
#include "engine.h"
#include "texture.h"
#include "maze/tile.h"
#include "maze/floodfill.h"
#include "util/list.h"
#include "util/util.h"

#define FX_EXPLODE_DURATION       300
#define FX_EXPLODE_SCALE          1.2
#define FX_FLOOD_DURADION         600
#define FX_FLOOD_INFLEXION_POINT  0.1

// ============ Door Effect ============ //

static void drawTriangle(vec2d vertexes[3]) {
  glBegin(GL_POLYGON);
  for (size_t i = 0; i < 3; i++) {
    glVertex2d(vertexes[i].x, vertexes[i].y);
  }
  glEnd();
}

static void fxDoorCloseRender(Animation* animation) {
  double percent = (double)animation->currentFrame / animation->frameCount;

  double width = GameState.ortho.width;
  double height = GameState.ortho.height;

  vec2d above[3] = {
    { 0.0, height },
    { width * percent, height },
    { 0.0, height * (1.0 - percent) },
  };

  vec2d below[3] = {
    { width, 0.0 },
    { width, height * percent},
    { width * (1.0 - percent), 0.0 },
  };

  glColor3d(1.0, 1.0, 1.0);
  drawTriangle(above);
  drawTriangle(below);
}

static void fxDoorOpenRender(Animation* animation) {
  double percent = (double)animation->currentFrame / animation->frameCount;

  double width = MAZE_SIZE;
  double height = MAZE_SIZE + HUD_HEIGHT;

  vec2d above[3] = {
    { 0.0, height },
    { width * (1.0 - percent), height },
    { 0.0, height * percent },
  };

  vec2d below[3] = {
    { width, 0.0 },
    { width, height * (1.0 - percent) },
    { width * percent, 0.0 },
  };

  glColor3d(1.0, 1.0, 1.0);
  drawTriangle(above);
  drawTriangle(below);
}

Animation* fxDoorOpen(double duration) {
  Animation* animation = createAnimation60FPS(duration, 1);

  animation->render = fxDoorOpenRender;

  return animation;
}

Animation* fxDoorClose(double duration) {
  Animation* animation = createAnimation60FPS(duration, 1);

  animation->render = fxDoorCloseRender;

  return animation;
}

// ============ Explode Effect ============ //

static void fxExplodeRender(Animation* animation) {
  int* row = animation->target;
  vec2i* start = animation->from;

  double offset = (1.0 - FX_EXPLODE_SCALE) / 2.0;

  renderSprite(FX_SPRITES, *row,
               animation->currentFrame - 1,
               start->x + offset, start->y + offset, // centering
               FX_EXPLODE_SCALE, FX_EXPLODE_SCALE);
}

Animation* fxExplode(int spriteRow, int x, int y) {
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

  return animation;
}

// ============ Flood Effect ============ //

typedef struct FxFloodRecord {
  FloodState* state;
  List* frontiers;
} FxFloodRecord;

static void fxFloodRender(Animation* animation);
static void fxFloodUpdate(Animation* animation);
static void fxFloodFinish(Animation* animation);

static void clearSpoiledTile(int x, int y) {
  if (GameState.maze[y][x] & TILE_SPOILED) {
    clearBits(GameState.maze[y][x], TILE_SPOILED);
    fxExplode(FX_SMOKE_ROW, x + 0.5, y + 0.5);
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
  record->frontiers = listClone(state->frontiers, sizeof(vec2i));

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
    Node* node = it.next(&it);
    vec2i* frontier = node->data;
    int x = frontier->x;
    int y = frontier->y;

    double ox = x + 0.5;
    double oy = y + 0.5;

    clearSpoiledTile(x, y);

    int row, col;

    Tile tile = GameState.maze[y][x];

    if (tile & TILE_COIN) {
      row = COIN_ROW;
      col = COIN_COL;
    } else if (tile & TILE_KERNEL) {
      row = FIRE_ROW;
      col = FIRE_COL;
    } else {
      row = COIN_DARK_ROW;
      col = COIN_DARK_COL;
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

void fxFlood(int x, int y) {
  FloodState* state = floodGenerate(GameState.maze, x, y);

  fxExplode(FX_EXPLODE_ROW, x, y);

  fxFloodNext(state);

  clearSpoiledTile(x, y);
}
