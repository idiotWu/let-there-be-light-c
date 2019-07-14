#ifndef GAME_KEYBOARD_H
#define GAME_KEYBOARD_H

#include "maze/direction.h"

void bindGameKeyboardHandlers(void);
void removeGameKeyboardHandlers(void);

Direction getPressedKeys(void);

#endif
