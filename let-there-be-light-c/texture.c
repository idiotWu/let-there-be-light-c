#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "util.h"
#include "texture.h"

#define IMG_FONT        "assets/font-16x16.bin"
#define IMG_PLAYER      "assets/fireball.bin"
#define IMG_MISC        "assets/misc.bin"
#define IMG_FX_EXPLODE  "assets/explode.bin"
#define IMG_ENERGY_BAR  "assets/energy-bar.bin"

#define ENERGY_BAR_FILL_OFFSET (12.0 / 256.0)

#define FOG_SIZE 128

Sprite* PLAYER_SPRITES;
Sprite* MISC_SPRITES;
Sprite* FX_EXPLODE_SPRITES;
Sprite* FONT_SPRITES;
Sprite* ENERGY_BAR_SPRITES;

static GLuint FOG_TEX;

static GLfloat TEX_PARAM = GL_REPLACE;

/**
 * Hex View <image.bin>:
 *   [0a 0b 0c 0d][1a 1b 1c 1d][ff ff ff ff ...]
 *   <-- width --><-- height -><--   pixels  -->
 */
static uint32_t readDimension(FILE* fp, uint64_t offset) {
  uint32_t dimension = 0;
  uint8_t byte;

  fseek(fp, offset, SEEK_SET);

  for (size_t i = 0; i < 4; i++) {
    fread(&byte, 1, 1, fp);

    // little-endian
    dimension += (byte << (i * 8));
  }

  return dimension;
}

static void bindTexture(GLuint* texName,
                        uint32_t width,
                        uint32_t height,
                        GLubyte* pixels) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, texName);
  glBindTexture(GL_TEXTURE_2D, *texName);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

static GLuint createTextureFrom(const char* filename) {
  GLuint texName;
  FILE* fp = fopen(filename, "r");

  if (fp == NULL) {
    fprintf(stderr, "cannot open file %s\n", filename);
    exit(1);
  }

  uint32_t width = readDimension(fp, 0);
  uint32_t height = readDimension(fp, 4);
  size_t byteLength = width * height * 4;

  GLubyte* pixels = malloc(byteLength);

  fseek(fp, 8, SEEK_SET);
  fread(pixels, 1, byteLength, fp);

  bindTexture(&texName, width, height, pixels);

  fclose(fp);
  free(pixels);

  return texName;
}

static GLuint initFogTexture() {
  GLuint texName;

  GLubyte pixels[FOG_SIZE * FOG_SIZE * 4];
  double radius = FOG_SIZE / 2.0;

  for (size_t i = 0; i < FOG_SIZE; i++) {
    for (size_t j = 0; j < FOG_SIZE; j++) {
      size_t offset = (i * FOG_SIZE + j) * 4;

      // rgb = (0, 0, 0)
      pixels[offset] = pixels[offset + 1] = pixels[offset + 2] = 0;

      double distance = sqrt(pow(i - radius, 2) + pow(j - radius, 2));

      // alpha
      pixels[offset + 3] = (GLubyte)(0xff * min(1.0, distance / radius));
    }
  }

  bindTexture(&texName, FOG_SIZE, FOG_SIZE, pixels);

  return texName;
}

// texture mapping
static void mapTexture(GLuint texName,
                       double sx, double sy,
                       double sw, double sh,
                       double dx, double dy,
                       double dw, double dh) {
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, TEX_PARAM);

  glBindTexture(GL_TEXTURE_2D, texName);
  glBegin(GL_QUADS);
    glTexCoord2d(sx, sy);
    glVertex2d(dx, dy + dh);

    glTexCoord2d(sx + sw, sy);
    glVertex2d(dx + dw, dy + dh);

    glTexCoord2d(sx + sw, sy + sh);
    glVertex2d(dx + dw, dy);

    glTexCoord2d(sx, sy + sh);
    glVertex2d(dx, dy);
  glEnd();

  glDisable(GL_TEXTURE_2D);
}

void setTexParam(GLfloat param) {
  TEX_PARAM = param;
}

void restoreDefaultTexParam(void) {
  TEX_PARAM = GL_REPLACE;
}

void renderSprite(Sprite* sprite,
                  uint32_t row, uint32_t col,
                  double dx, double dy,
                  double dw, double dh) {
  double sw = 1.0 / sprite->cols;
  double sh = 1.0 / sprite->rows;
  double sx = (double)col * sw;
  double sy = (double)row * sh;

  mapTexture(sprite->texture,
             sx, sy, sw, sh,
             dx, dy, dw, dh);
}

void renderFog(double dx, double dy,
               double dw, double dh) {
  mapTexture(FOG_TEX,
             0.0, 0.0, 1.0, 1.0,
             dx, dy, dw, dh);
}

void renderEnergyBar(double percent,
                     double dx, double dy,
                     double dw, double dh) {
  double sw = 1.0 / ENERGY_BAR_SPRITES->cols;
  double sh = 1.0 / ENERGY_BAR_SPRITES->rows;

  int row = (int)ceil(percent * (ENERGY_BAR_SPRITES->rows - 1));
  double fillWidth = (1.0 - ENERGY_BAR_FILL_OFFSET * 2.0) * percent;

  // background
  mapTexture(ENERGY_BAR_SPRITES->texture, 0.0, 0.0, sw, sh, dx, dy, dw, dh);

  // fill
  mapTexture(ENERGY_BAR_SPRITES->texture,
             sw * ENERGY_BAR_FILL_OFFSET, row * sh,
             sw * fillWidth, sh,
             dx + dw * ENERGY_BAR_FILL_OFFSET, dy,
             dw * fillWidth, dh);
}

void renderText(const char* str,
                double x, double y,
                double fontSize) {
  glPushMatrix();
  glTranslated(x, y, 0.0);

  for (size_t i = 0; i < strlen(str); i++) {
    int offset = str[i] - ' ';

    renderSprite(FONT_SPRITES,
                 offset / FONT_SPRITES->cols,
                 offset % FONT_SPRITES->cols,
                 i * fontSize, 0, fontSize, fontSize);
  }

  glPopMatrix();
}

void initTextures(void) {
  PLAYER_SPRITES = malloc(sizeof(Sprite));
  MISC_SPRITES = malloc(sizeof(Sprite));
  FX_EXPLODE_SPRITES = malloc(sizeof(Sprite));
  FONT_SPRITES = malloc(sizeof(Sprite));
  ENERGY_BAR_SPRITES = malloc(sizeof(Sprite));

  PLAYER_SPRITES->texture = createTextureFrom(IMG_PLAYER);
  PLAYER_SPRITES->rows = 4;
  PLAYER_SPRITES->cols = 4;

  MISC_SPRITES->texture = createTextureFrom(IMG_MISC);
  MISC_SPRITES->rows = 4;
  MISC_SPRITES->cols = 2;

  FX_EXPLODE_SPRITES->texture = createTextureFrom(IMG_FX_EXPLODE);
  FX_EXPLODE_SPRITES->rows = 1;
  FX_EXPLODE_SPRITES->cols = 5;

  FONT_SPRITES->texture = createTextureFrom(IMG_FONT);
  FONT_SPRITES->rows = 8;
  FONT_SPRITES->cols = 15;

  ENERGY_BAR_SPRITES->texture = createTextureFrom(IMG_ENERGY_BAR);
  ENERGY_BAR_SPRITES->rows = 5;
  ENERGY_BAR_SPRITES->cols = 1;

  FOG_TEX = initFogTexture();
}
