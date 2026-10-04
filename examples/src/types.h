#ifndef _TYPES_H
#define _TYPES_H
#include "raylib.h"

/// cmy:reflect
typedef struct
{
    Vector2 pos;
    Vector2 speed;
    float radius;
    Color color;
} Ball;

/// cmy:reflect
typedef struct
{
    int bg_shade;
    Color ball_color;
} Theme;

/// cmy:reflect
typedef enum
{
    GAME_PLAYING,
    GAME_PAUSED,
} GameState;

#define MAX_BALLS 9

/// cmy:reflect
typedef struct
{
    Theme theme;
    GameState state;
    int score;

    Ball balls[MAX_BALLS];
} Game;

#endif // _TYPES_H
