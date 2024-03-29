/**
 * @file
 * @brief テクスチャ
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "glut.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "render/texture.h"
#include "util/util.h"

#define IMG_FONT            "assets/font-16x16.bin"
#define IMG_PLAYER          "assets/fireball.bin"
#define IMG_PLAYER_SPOILED  "assets/fireball-green.bin"
#define IMG_ENEMY           "assets/enemy.bin"
#define IMG_MISC            "assets/misc.bin"
#define IMG_FX              "assets/fx.bin"
#define IMG_ENERGY_BAR      "assets/energy-bar.bin"

//! エネルギーバーの内部のオフセット
#define ENERGY_BAR_FILL_OFFSET (12.0 / 256.0)

//! 霧のテクスチャのサイズ
#define FOG_SIZE 128

Sprite* PLAYER_SPRITES;
Sprite* PLAYER_SPRITES_SPOILED;
Sprite* ENEMY_SPRITES;
Sprite* MISC_SPRITES;
Sprite* FX_SPRITES;
Sprite* FONT_SPRITES;
Sprite* ENERGY_BAR_SPRITES;

/**
 * @internal
 * @brief 霧のテクスチャ
 */
static GLuint FOG_TEX;

/**
 * @internal
 * @brief テクスチャパラメータの初期値
 */
static GLfloat TEX_PARAM = GL_REPLACE;

/**
 * @brief バイナリデータから画像の長さ，または高さを読み取る
 *
 * @details バイナリデータの仕様：
 * ```
 * Hex View <image.bin>:
 *   [0a 0b 0c 0d][1a 1b 1c 1d][ff ff ff ff ...]
 *   <-- width --><-- height -><--   pixels  -->
 * ```
 *
 * @param fp     ファイルポインタ
 * @param offset データのオフセット
 *
 * @return uint32_t 画像の長さ，または高さ
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

/**
 * @brief テクスチャを有効にする
 *
 * @param texName テクスチャの ID
 * @param width   テクスチャの長さ
 * @param height  テクスチャの高さ
 * @param pixels  ピクセルデータ
 */
static void bindTexture(GLuint* texName,
                        uint32_t width,
                        uint32_t height,
                        GLubyte* pixels) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, texName);
  glBindTexture(GL_TEXTURE_2D, *texName);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

/**
 * @brief バイナリデータのファイルからテクスチャを生成する
 *
 * @param filename バイナリデータのファイル名
 *
 * @return GLuint テクスチャの ID
 */
static GLuint createTextureFrom(const char* filename) {
  GLuint texName;
  FILE* fp = fopen(filename, "r");

  if (fp == NULL) {
    char msg[256];

    sprintf(msg, "cannot open file %s", filename);

#ifdef _WIN32
    MessageBoxA(NULL, msg, "Error", MB_OK);
#else
    fprintf(stderr, "%s\n", msg);
#endif
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

/**
 * @brief 霧のテクスチャを生成する
 *
 * @return GLuint 霧のテクスチャの ID
 */
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

/**
 * @brief テクスチャをマッピングする
 *
 * @param texName テクスチャの ID
 * @param sx      マッピングしたいテクスチャ空間の x 座標
 * @param sy      マッピングしたいテクスチャ空間の y 座標
 * @param sw      マッピングしたいテクスチャ空間の長さ
 * @param sh      マッピングしたいテクスチャ空間の高さ
 * @param dx      出力先の x 座標
 * @param dy      出力先の y 座標
 * @param dw      出力先の長さ
 * @param dh      出力先の高さ
 */
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

void loadTextures(void) {
  PLAYER_SPRITES = malloc(sizeof(Sprite));
  PLAYER_SPRITES_SPOILED = malloc(sizeof(Sprite));
  ENEMY_SPRITES = malloc(sizeof(Sprite));
  MISC_SPRITES = malloc(sizeof(Sprite));
  FX_SPRITES = malloc(sizeof(Sprite));
  FONT_SPRITES = malloc(sizeof(Sprite));
  ENERGY_BAR_SPRITES = malloc(sizeof(Sprite));

  PLAYER_SPRITES->texture = createTextureFrom(IMG_PLAYER);
  PLAYER_SPRITES->rows = 4;
  PLAYER_SPRITES->cols = 4;

  PLAYER_SPRITES_SPOILED->texture = createTextureFrom(IMG_PLAYER_SPOILED);
  PLAYER_SPRITES_SPOILED->rows = 4;
  PLAYER_SPRITES_SPOILED->cols = 4;


  ENEMY_SPRITES->texture = createTextureFrom(IMG_ENEMY);
  ENEMY_SPRITES->rows = 4;
  ENEMY_SPRITES->cols = 4;

  MISC_SPRITES->texture = createTextureFrom(IMG_MISC);
  MISC_SPRITES->rows = 4;
  MISC_SPRITES->cols = 2;

  FX_SPRITES->texture = createTextureFrom(IMG_FX);
  FX_SPRITES->rows = 5;
  FX_SPRITES->cols = 5;

  FONT_SPRITES->texture = createTextureFrom(IMG_FONT);
  FONT_SPRITES->rows = 11;
  FONT_SPRITES->cols = 11;

  ENERGY_BAR_SPRITES->texture = createTextureFrom(IMG_ENERGY_BAR);
  ENERGY_BAR_SPRITES->rows = 5;
  ENERGY_BAR_SPRITES->cols = 1;

  FOG_TEX = initFogTexture();
}
