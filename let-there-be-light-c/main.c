#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glut.h"

#include "engine.h"
#include "util.h"

#define UNUSED(x) (void)(x)
#define RECT_COUNT 10
#define MAX_HEIGHT 1.5
#define RECT_WIDTH (2.0 / RECT_COUNT)
#define RECT_HEIGHT_STEP (MAX_HEIGHT / (RECT_COUNT / 2.0))

typedef struct Rectangle {
  double x1;
  double y1;
  double x2;
  double y2;
  double color[3];
} Rectangle;

double easeInOutBack(double pos) {
  double s = 1.70158;
  if((pos/=0.5) < 1) return 0.5*(pos*pos*(((s*=(1.525))+1)*pos -s));
  return 0.5*((pos-=2)*pos*(((s*=(1.525))+1)*pos +s) +2);
}

void renderRect(Animation* animation) {
  Rectangle* rect = animation->shape;
  
  double percent = (double)animation->currentFrame / animation->totalFrames;
  
  double pFix = percent < 0.5 ? (percent * 2) : (1 - (percent - 0.5) * 2);
  
  double pos = easeInOutBack(pFix);
  
  if (animation->currentFrame == 0) {
    rect->color[0] = randomBetween(0, 1);
    rect->color[1] = randomBetween(0, 1);
    rect->color[2] = randomBetween(0, 1);
  }
  
  glColor3dv(rect->color);
  glRectd(
          rect->x1,
          rect->y1,
          rect->x2,
          rect->y1 + (rect->y2 - rect->y1) * pos);
}

double calcHeight(unsigned int i) {
  unsigned int m = RECT_COUNT / 2;
  
  return i < m ? RECT_HEIGHT_STEP * (i + 1) : (MAX_HEIGHT - RECT_HEIGHT_STEP * (i - m));
}

void createRect(Animation* animation) {
  unsigned int interval = animation->totalFrames / RECT_COUNT;
  if (animation->currentFrame % interval == 0) {
    Rectangle* rect = malloc(sizeof(Rectangle));
    unsigned int i = animation->currentFrame / interval;
    double height = calcHeight(i);
    
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
