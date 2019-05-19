#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glut.h"

#include "engine.h"
#include "util.h"

#define UNUSED(x) (void)(x)
#define RECT_COUNT 11
#define RECT_WIDTH (2.0 / RECT_COUNT)

typedef struct Rectangle {
  double x1;
  double y1;
  double x2;
  double y2;
  double color[3];
} Rectangle;

double easeInOutBack(double p) {
  if (p < 0.5) {
    double f = 2 * p;
    return 0.5 * (f * f * f - f * sin(f * M_PI));
  } else {
    double f = (1 - (2*p - 1));
    return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
  }
}

void renderRect(Animation* animation) {
  Rectangle* rect = animation->shape;
  
  double percent = (double)animation->currentFrame / animation->totalFrames;
  
  double pFix = percent < 0.5 ? (percent * 2) : (1 - (percent - 0.5) * 2);
  
  double pos = easeInOutBack(pFix);
  
  if (animation->currentFrame == 0) {
    rect->color[0] = randomBetween(0, 1);
    rect->color[1] = randomBetween(0.5, 1);
    rect->color[2] = randomBetween(0.5, 1);
  }
  
  glColor3dv(rect->color);
  glRectd(
          rect->x1,
          rect->y1,
          rect->x2,
          rect->y1 + (rect->y2 - rect->y1) * pos);
}

void createRect(Animation* animation) {
  unsigned int interval = animation->totalFrames / (RECT_COUNT - 1);
  
  if (animation->currentFrame % interval == 0) {
    Rectangle* rect = malloc(sizeof(Rectangle));
    
    unsigned int i = animation->currentFrame / interval;
    
    double height = sin((double)animation->currentFrame / animation->totalFrames * M_PI) * 1.25 + 0.25;
    
    rect->x1 = -1 + RECT_WIDTH * i;
    rect->y1 = -1;
    rect->x2 = rect->x1 + RECT_WIDTH;
    rect->y2 = -1 + height;
    
    Animation* a = createAnimation60FPS(1500, ANIMATION_INFINITY);
    
    a->shape = rect;
    a->render = renderRect;
  }
}

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
  
  Animation* animation = createAnimation60FPS(1000, 1);
  
  animation->render = createRect;
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
