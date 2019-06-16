#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "glut.h"

#include "config.h"
#include "engine.h"
#include "util.h"
#include "texture.h"

#define UNUSED(x) (void)(x)


void init(void) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  initTextures();

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
  gluOrtho2D(0.0, MAZE_SIZE, 0.0, MAZE_SIZE);
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
