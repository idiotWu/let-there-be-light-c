#include "glut.h"

#include "keyboard.h"

#include "util/util.h"
#include "maze/direction.h"

static Direction keyPressed = DIR_NONE;

// map the given key code to a direction
static Direction keyToDirection(int key) {
  switch (key) {
    case GLUT_KEY_UP:
      return DIR_UP;

    case GLUT_KEY_RIGHT:
      return DIR_RIGHT;

    case GLUT_KEY_DOWN:
      return DIR_DOWN;

    case GLUT_KEY_LEFT:
      return DIR_LEFT;

    default:
      return DIR_NONE;
  }
}

static void keydownHandler(int key, int x, int y) {
  UNUSED(x); UNUSED(y);
  setBits(keyPressed, keyToDirection(key));
}

static void keyupHandler(int key, int x, int y) {
  UNUSED(x); UNUSED(y);
  clearBits(keyPressed, keyToDirection(key));
}

void bindGameKeyboardHandlers(void) {
  keyPressed = DIR_NONE;

  glutSpecialFunc(keydownHandler);
  glutSpecialUpFunc(keyupHandler);
}

void removeGameKeyboardHandlers(void) {
  keyPressed = DIR_NONE;

  glutSpecialFunc(NULL);
  glutSpecialUpFunc(NULL);
}

Direction getPressedKeys(void) {
  return keyPressed;
}
