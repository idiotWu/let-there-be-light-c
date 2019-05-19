#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glut.h"

#include "engine.h"

#define UNUSED(x) (void)(x)

void render(Animation* animation) {
  double percent = (double)animation->currentFrame / animation->totalFrames;
  
  double c = percent < 0.5 ? (percent * 2) : (1 - (percent - 0.5) * 2);
  
  printf("%2d/%2d\t%.2lf\n", animation->currentFrame, animation->totalFrames, c);
  
  glColor3d(0.75 + c * 0.25, c, 1 - c);
  glRectd(-1.0 + c, -0.5, c, 0.5);
}

void init(void) {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  
  Animation* animation = createAnimation60FPS(1000, ANIMATION_INFINITY);
  
  animation->render = render;
}

void update(int _) {
  glutPostRedisplay();
  
  UNUSED(_);
  
  glutTimerFunc(ANIMATION_60_FPS, update, 0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  engineNext();
  glutSwapBuffers();
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutCreateWindow("Test");
  glutDisplayFunc(display);
  init();
  update(0);
  glutMainLoop();
  
  return EXIT_SUCCESS;
}
