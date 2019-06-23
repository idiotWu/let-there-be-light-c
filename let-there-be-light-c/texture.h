#ifndef TEXTURE_H
#define TEXTURE_H

#include "glut.h"

#define PLAYER_FRONT_ROW 0
#define PLAYER_BACK_ROW  1
#define PLAYER_LEFT_ROW  2
#define PLAYER_RIGHT_ROW 3

#define PATH_ROW 0
#define PATH_COL 0

#define SHADOW_ROW 0
#define SHADOW_COL 1

#define FIRE_ROW 1
#define FIRE_COL 0

#define COIN_ROW 1
#define COIN_COL 1

#define COIN_DARK_ROW 2
#define COIN_DARK_COL 0

#define COIN_BIG_ROW 2
#define COIN_BIG_COL 1

#define SPACESHIP_ROW 3
#define SPACESHIPT_COL 0

#define BULLET_ROW 3
#define BULLET_COL 1


typedef struct Sprite {
  GLuint texture;
  GLuint rows;
  GLuint cols;
} Sprite;

extern Sprite* PLAYER_SPRITES;
extern Sprite* MISC_SPRITES;
extern Sprite* FX_EXPLODE_SPRITES;
extern Sprite* FONT_SPRITES;
extern Sprite* ENERGY_BAR_SPRITES;

void setTextureAlpha(double alpha);

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
