#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "util.h"
#include "texture.h"

#define IMG_PLAYER  "assets/fireball.bin"
#define IMG_MISC    "assets/misc.bin"

#define FOG_SIZE    128

#define PLAYER_ROW_CNT  4
#define PLAYER_COL_CNT  4
#define MISC_ROW_CNT    3
#define MISC_COL_CNT    2

GLuint PLAYER_SPRITES;
GLuint MISC_SPRITES;
GLuint FOG_TEX;

static uint32_t readDimension(FILE* fp, uint64_t offset) {
  uint32_t dimension = 0;
  uint8_t byte;

  fseek(fp, offset, SEEK_SET);

  for (size_t i = 0; i < 4; i++) {
    fread(&byte, 1, 1, fp);

    // little-endian
    dimension += (byte << i);
  }

  return dimension;
}

static void bindTexture(GLuint* texName,
                        uint32_t width,
                        uint32_t height,
                        GLbyte* pixels) {
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

static void createTextureFrom(const char* filename, GLuint* texName) {
  FILE* fp = fopen(filename, "r");

  if (fp == NULL) {
    fprintf(stderr, "cannot open file %s\n", filename);
    exit(1);
  }

  uint32_t width = readDimension(fp, 0);
  uint32_t height = readDimension(fp, 4);

  GLbyte* pixels = malloc(4 * width * height);

  fseek(fp, 8, SEEK_SET);
  fread(pixels, 1, 4 * width * height, fp);

  bindTexture(texName, width, height, pixels);

  free(pixels);
}

static void initFogTexture(GLuint* texName) {
  GLbyte pixels[FOG_SIZE * FOG_SIZE * 4];
  double radius = FOG_SIZE / 2.0;

  for (size_t i = 0; i < FOG_SIZE; i++) {
    for (size_t j = 0; j < FOG_SIZE; j++) {
      size_t offset = (i * FOG_SIZE + j) * 4;

      // rgb = (0, 0, 0)
      pixels[offset] =
      pixels[offset + 1] =
      pixels[offset + 2] = 0;

      double distance = sqrt(pow(i - radius, 2) + pow(j - radius, 2));

      pixels[offset + 3] = (GLbyte)(0xff * min(1.0, distance / radius));
    }
  }

  bindTexture(texName, FOG_SIZE, FOG_SIZE, pixels);
}

// texture mapping
static void mapTexture(GLuint texName,
                       double sx, double sy,
                       double sw, double sh,
                       double dx, double dy,
                       double dw, double dh) {
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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

void renderSprite(GLuint texName,
                  uint32_t row, uint32_t col,
                  double dx, double dy,
                  double dw, double dh) {
  double sw, sh;

  if (texName == PLAYER_SPRITES) {
    sw = 1.0 / PLAYER_COL_CNT;
    sh = 1.0 / PLAYER_ROW_CNT;
  } else if (texName == MISC_SPRITES) {
    sw = 1.0 / MISC_COL_CNT;
    sh = 1.0 / MISC_ROW_CNT;
  } else {
    fprintf(stderr, "unknown texture ID: %u", texName);
    return;
  }

  double sx = (double)col * sw;
  double sy = (double)row * sh;

  mapTexture(texName,
             sx, sy, sw, sh,
             dx, dy, dh, dw);
}

void renderFog(double dx, double dy,
               double dw, double dh) {
  mapTexture(FOG_TEX,
             0.0, 0.0, 1.0, 1.0,
             dx, dy, dh, dw);
}

void initTextures(void) {
  createTextureFrom(IMG_PLAYER, &PLAYER_SPRITES);
  createTextureFrom(IMG_MISC, &MISC_SPRITES);
  initFogTexture(&FOG_TEX);
}
