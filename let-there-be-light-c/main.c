#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glut.h"

#include "engine.h"
#include "util.h"

//#define RENDER_RECT
#define RENDER_WAVE
#define LISTEN_MOUSE_STAR

#define deg2rad(x) (x / 180.0 * M_PI)
#define UNUSED(x) (void)(x)

#define RECT_COUNT 60
#define RECT_WIDTH (2.0 / RECT_COUNT)

typedef struct Rectangle {
  double x1;
  double y1;
  double x2;
  double y2;
  double color[3];
} Rectangle;

typedef struct Star {
  double x;
  double y;
  double r;
  double rot;
  double color[4];
} Star;

double easeInOutBack(double p) {
  if (p < 0.5) {
    double f = 2 * p;
    return 0.5 * (f * f * f - f * sin(f * M_PI));
  } else {
    double f = (1 - (2*p - 1));
    return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
  }
}

void renderWave(Animation* animation) {
  Rectangle* rect = animation->from;

  double percent = (double)animation->currentFrame / animation->frameCount;

  double pFix = percent < 0.5 ? (percent * 2) : (1 - (percent - 0.5) * 2);

  double pos = easeInOutBack(pFix);

  if (animation->currentFrame == 1) {
    rect->color[0] = randomBetween(0.5, 1);
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

static int i = -1;

void createWave(Animation* animation) {
  double interval = (double)(animation->frameCount) / RECT_COUNT;

  int t = animation->currentFrame / interval;

  if (t > i) {
    i = t;

    Rectangle* rect = malloc(sizeof(Rectangle));

    double offset = RECT_WIDTH * (int)((animation->currentFrame - 1) / interval);

    double height = sin((double)animation->currentFrame / (animation->frameCount) * M_PI) * 1.0 + 0.25;

    rect->x1 = -1.0 + offset;
    rect->y1 = -1.0;
    rect->x2 = rect->x1 + RECT_WIDTH;
    rect->y2 = -1.0 + height;

    Animation* a = createAnimation60FPS(1500, ANIMATION_INFINITY);

    a->from = rect;
    a->render = renderWave;
  }
}

void renderRect(Animation* animation) {
  double percent = (double)animation->currentFrame / animation->frameCount;

  double c = percent < 0.5 ? (percent * 2) : (1 - (percent - 0.5) * 2);

  printf("%2d/%2d\t%.2lf\n", animation->currentFrame, animation->frameCount, c);

  glColor3d(0.75 + c * 0.25, c, 1 - c);
  glRectd(-1.0 + c, -0.5, c, 0.5);
}

const double starVertexes[5] = { 90.0, 234.0, 18.0, 162.0, 306.0 };

void renderStar(Animation* animation) {
  Star* star = animation->from;

  star->r *= 0.95;
  star->y -= 1.0/60.0;
  star->rot += 10.0;
  star->color[3] = 1.0 - (double)animation->currentFrame / animation->frameCount;

  double scaleX = (double)glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT);

  glColor4dv(star->color);

  glBegin(GL_TRIANGLE_FAN);
    glScaled(10.0, 1.0, 1.0);
    glVertex2d(star->x, star->y);

    for (size_t i = 0; i <= 5; i++) {
      double rad = deg2rad(star->rot +starVertexes[i % 5]);
      glVertex2d(star->x + star->r / scaleX * cos(rad), star->y + star->r * sin(rad));
    }
  glEnd();
}

void mouseListenerStar(int x, int y) {
  double u = (double)x / glutGet(GLUT_WINDOW_WIDTH) * 2.0 - 1.0;
  double v = -(double)y / glutGet(GLUT_WINDOW_HEIGHT) * 2.0 + 1.0;

  Star* star = malloc(sizeof(Star));

  star->x = u;
  star->y = v;
  star->r = randomBetween(0.05, 0.1);
  star->rot = randomBetween(0.0, 360.0);
  star->color[0] = randomBetween(0.5, 1);
  star->color[1] = randomBetween(0.5, 1);
  star->color[2] = randomBetween(0.5, 1);

  Animation* animation = createAnimation60FPS(2000, 1);
  animation->from = star;
  animation->render = renderStar;
}

void init(void) {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(0.0, 0.0, 0.0, 0.0);

#ifdef RENDER_RECT
  Animation* animation = createAnimation60FPS(1000, ANIMATION_INFINITY);

  animation->render = renderRect;
#endif

#ifdef RENDER_WAVE
  Animation* animation = createAnimation60FPS(1000, 1);

  animation->render = createWave;
#endif

#ifdef LISTEN_MOUSE_STAR
  glutMotionFunc(mouseListenerStar);
  glutPassiveMotionFunc(mouseListenerStar);
#endif
}

void update(int _) {
  glutPostRedisplay();

  UNUSED(_);

  glutTimerFunc(ANIMATION_60_FPS, update, 0);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
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
