#include "glut.h"

#include "render/helper.h"

void clipRect(double x, double y, double width, double height) {
  // enable writing to the stencil buffer
  glStencilMask(0xff);
  // Clear stencil buffer
  glClear(GL_STENCIL_BUFFER_BIT);
  // discard pixels out of stencil area
  glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
  // all fragments should update the stencil buffer
  glStencilFunc(GL_ALWAYS, 1, 0xff);

  // draw a rectangle into stencil
  glColor4d(0.0, 0.0, 0.0, 0.0);
  glRectd(x, y, x + width, y + height);

  // disable writing to the stencil buffer
  glStencilMask(0);

  // only keeps pixels inside the rectangle area
  glStencilFunc(GL_EQUAL, 1, 0xff);
}
