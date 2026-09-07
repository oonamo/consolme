#ifndef _TYPES_H
#define _TYPES_H
#include "raylib.h"

/// @reflect
typedef struct
{
    float pos;
    int dir;
} Player;

/// @reflect
typedef struct
{
    int level;
    Color background_color;
    Player player;
} Game;

#endif // _TYPES_H
