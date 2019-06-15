#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "glut.h"

#include "engine.h"
#include "util.h"
#include "texture.h"

#define UNUSED(x) (void)(x)

void renderFireBall(Animation* animation) {
  uint32_t n = animation->currentFrame - 1;
  uint32_t row = n / 4;
  uint32_t col = n % 4;

  printf("(%u, %u)\n", row, col);

  // path
  renderSprite(MISC_SPRITES, 0, 0, -0.25, 0.0, 0.5, 0.5);
  // shadow
  renderSprite(MISC_SPRITES, 0, 1, -0.25, -0.5, 0.5, 0.5);
  // player
  renderSprite(PLAYER_SPRITES, row, col, -0.25, 0.0, 0.5, 0.5);

  // fog
  renderFog(-0.25, 0.0, 0.5, 0.5);
}

void init(void) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  initTextures();

  Animation* animation = createAnimation(16, 3e3, ANIMATION_INFINITY);

  animation->render = renderFireBall;
}

void update(int _) {
  UNUSED(_);
  glutPostRedisplay();
  glutTimerFunc(ANIMATION_60_FPS, update, 0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  engineNextFrame();

  glutSwapBuffers();
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(800, 800);
  glutCreateWindow("Test");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  init();
  update(0);
  glutMainLoop();

  return EXIT_SUCCESS;
}
