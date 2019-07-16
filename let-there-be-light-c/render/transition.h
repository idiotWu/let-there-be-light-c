#ifndef TRANSITION_H
#define TRANSITION_H

#include "scene/scene.h"
#include "render/engine.h"

Animation* transitionIn(double duration);
Animation* transitionOut(double duration);

void transitionQueue(double duration, Scene* target, double rest, Scene* midway);
void levelTransition(void);

#endif
