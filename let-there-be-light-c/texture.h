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

#define SPACESHIP_ROW 2
#define SPACESHIPT_COL 0

#define BULLET_ROW 2
#define BULLET_COL 1

extern GLuint PLAYER_SPRITES;
extern GLuint MISC_SPRITES;

void renderSprite(GLuint texName,
                  uint32_t row, uint32_t col,
                  double dx, double dy,
                  double dw, double dh);

void renderFog(double dx, double dy,
               double dw, double dh);

void initTextures(void);

#endif
