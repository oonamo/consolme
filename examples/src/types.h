#ifndef _TYPES_H
#define _TYPES_H
#include "raylib.h"

#ifdef __REFLECTION_PARSER__

/// @reflect
typedef struct
{
    float x;
    float y;
} Vector2;

/// @reflect
typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

#endif // __REFLECTION_PARSER__

/// @reflect
typedef struct
{
    Vector2 pos;
    Vector2 speed;
    float radius;
    Color color;
} Ball;

/// @reflect
typedef struct
{
    int bg_shade;
    Color ball_color;
} Theme;

/// @reflect
typedef enum
{
    GAME_PLAYING,
    GAME_PAUSED,
} GameState;

#define MAX_BALLS 9

/// @reflect
typedef struct
{
    Theme theme;
    GameState state;
    int score;

    Ball balls[MAX_BALLS];
} Game;

#endif // _TYPES_H
