//
//  main.cpp
//  let-there-be-light-c
//
//  Created by Dolphin Wood on 2019/04/07.
//  Copyright © 2019 Dolphin Wood. All rights reserved.
//

#include <iostream>
#include "lib/gl.h"

const GLfloat lightPosition1[4] = {0.0f,3.0f, 5.0f, 1.0f};
const GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 };
const GLfloat lightPosition2[4] = {5.0f,3.0f, 0.0f, 1.0f};
const GLfloat red[] = { 1.0, 0.0, 0.0, 1.0 };
const GLfloat teapotAmbient[4] = {0.3f,0.5f, 0.0f, 1.0f};
const GLfloat teapotDiffuse[4] = {1.0f,1.0f, 0.3f, 1.0f};
const GLfloat teapotSpecular[4] = {1.0f,1.0f, 1.0f, 1.0f};
const GLfloat teapotShininess[4] = {20.0f};

void setup(void) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, red);
  glLightfv(GL_LIGHT0, GL_SPECULAR, red);
  glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, green);
  glLightfv(GL_LIGHT1, GL_SPECULAR, green);
  glMaterialfv(GL_FRONT, GL_AMBIENT, teapotAmbient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, teapotDiffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, teapotSpecular);
  glMaterialfv(GL_FRONT, GL_SHININESS, teapotShininess);
}

void draw(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glutSolidTeapot(0.5);
  glFlush();
}


void resize(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0,
                 (double)width/height,
                 0.1,
                 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(-0.5, 2.1, 2.0,
            0.0, 0.0, 0.0,
            0.0, 4.0, 0.0);
}

void display(void){
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glutWireTeapot(0.5);
  glFlush();
}


int main(int argc, char * argv[]) {
  glutInit(&argc, argv);
  glutInitWindowSize(600,600);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
  glutCreateWindow("Wire_teapot");
  glutReshapeFunc(resize);
  glutDisplayFunc(draw);
  setup();
  glutMainLoop();
  
  return 0;
}
