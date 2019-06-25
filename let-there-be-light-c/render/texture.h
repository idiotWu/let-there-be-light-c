#ifndef TEXTURE_H
#define TEXTURE_H

#include "glut.h"

#define CHARACTER_FRONT_ROW 0
#define CHARACTER_BACK_ROW  1
#define CHARACTER_LEFT_ROW  2
#define CHARACTER_RIGHT_ROW 3

#define ENEMY_INACTIVE_COL 3

#define PATH_ROW 0
#define PATH_COL 0

#define SHADOW_ROW 0
#define SHADOW_COL 1

#define FROZEN_PATH_ROW 1
#define FROZEN_PATH_COL 0

#define ICE_SPIKE_ROW 1
#define ICE_SPIKE_COL 1

#define FIRE_ROW 2
#define FIRE_COL 0

#define COIN_ROW 2
#define COIN_COL 1

#define COIN_DARK_ROW 3
#define COIN_DARK_COL 0

#define COIN_BIG_ROW 3
#define COIN_BIG_COL 1

#define FX_EXPLODE_ROW 0
#define FX_ICE_SPLIT_ROW 1
#define FX_ENEMY_SPAWN_ROW 2
#define FX_ENEMY_REVIVE_ROW 3
#define FX_SMOKE_ROW 4

typedef struct Sprite {
  GLuint texture;
  GLuint rows;
  GLuint cols;
} Sprite;

extern Sprite* PLAYER_SPRITES;
extern Sprite* PLAYER_SPRITES_SPOILED;
extern Sprite* ENEMY_SPRITES;
extern Sprite* MISC_SPRITES;
extern Sprite* FX_SPRITES;
extern Sprite* FONT_SPRITES;
extern Sprite* ENERGY_BAR_SPRITES;

void setTexParam(GLfloat param);
void restoreDefaultTexParam(void);

void renderSprite(Sprite* sprite,
                  uint32_t row, uint32_t col,
                  double dx, double dy,
                  double dw, double dh);

void renderFog(double dx, double dy,
               double dw, double dh);

void renderEnergyBar(double percent,
                     double dx, double dy,
                     double dw, double dh);

void renderText(const char* str,
                double x, double y,
                double fontSize);

void initTextures(void);

#endif
