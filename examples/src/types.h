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

/// @reflect
typedef struct
{
    Ball ball;
    Theme theme;
    GameState state;
    int score;
} Game;

#endif // _TYPES_H
