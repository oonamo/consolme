/*****************************************
 *
 * consolme - Basic Raylib Showcase
 *
 * This example demonstrates how to integrate:
 *  - CMyReflection (https://github.com/oonamo/CMyReflection)
 *  - consolme      (TODO: add consolme repo)
 *  - raylib        (https://github.com/raysan5/raylib)
 *****************************************/

//===================================================================
// Include the types then include the generated reflection file
//===================================================================

// clang-format off
#include "types.h"
#include "refl.generated.h"
// clang-format on

// Define implementation and extensions before using the library
#define CONSOLME_IMPLEMENTATION
#define CONSOLME_EXTENSION_CMYREFLECTION
#include <consolme.h>

#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

static const int width = 1600;
static const int height = 800;

//----------------------------------------
// Global Game State
//----------------------------------------
Game g = {.state = GAME_PLAYING,
          .main_ball = {.pos = {width / 2.0f, height / 2.0f},
                        .speed = {200.0f, 150.0f},
                        .radius = 40.0f},
          .theme = {
              30,
              .ball_color = GRAY,
          }};

//----------------------------------------
// Type Handlers & Macros
//----------------------------------------

// These macros generate functions that allow for converting a string to their
// respective struct
DEFINE_COLOR_SETTER(Set_Color);
DEFINE_COLOR_GETTER(Get_Color);

DEFINE_FLOAT2_SETTER(Set_Vector2, Vector2, x, y);
DEFINE_FLOAT2_GETTER(Get_Vector2, Vector2, x, y);

// Routes type-specific setters to their generated macros
ConsoleResponse GameTypeSetter(void *target_struct, const FieldInfo *leaf,
                               void *field_ptr, int argc, char **argv,
                               char *response_msg)
{

    switch (leaf->type)
    {
    case TYPE_STRUCT_COLOR:
        return Set_Color(field_ptr, argc, argv, response_msg);
    case TYPE_STRUCT_VECTOR2:
        return Set_Vector2(field_ptr, argc, argv, response_msg);
    default:
        snprintf(response_msg, MAX_INPUT_CHARS,
                 "Type '%s' was not implmeneted!",
                 get_name_of_type(leaf->type));
        return CMD_ERROR();
    }
}

// Routes type-specific getters to theire generated macros
ConsoleResponse GameTypeGetter(void *target_struct, const FieldInfo *leaf,
                               void *field_ptr, int argc, char **argv,
                               char *response_msg)
{
    switch (leaf->type)
    {
    case TYPE_STRUCT_VECTOR2:
        return Get_Vector2(field_ptr, response_msg);
    case TYPE_STRUCT_COLOR:
        return Get_Color(field_ptr, response_msg);
    default:
        snprintf(response_msg, MAX_INPUT_CHARS,
                 "Type '%s' was not implmeneted!",
                 get_name_of_type(leaf->type));
        return CMD_ERROR();
    }
}

// Command router. Controls the actual execution and result of the commands
ConsoleResponse GameConsole(const char *command, void *user_data,
                            char *response_msg)
{
    // Hack to not use global context incase scope changes;
    Game *active_game = (Game *)user_data;

    char cmd_copy[MAX_INPUT_CHARS];
    strncpy(cmd_copy, command, MAX_INPUT_CHARS);
    cmd_copy[MAX_INPUT_CHARS - 1] = '\0';

    char *argv[10];
    int argc = Console_Tokenize(cmd_copy, argv, 10);
    if (argc == 0) return CMD_ERROR();

    // Route dynamic reflection setters ("set theme.ball_color 0xFF0000FF")
    if (strcmp(argv[0], "set") == 0)
    {
        if (argc < 3)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Usage: set <path> <value>");
            return CMD_ERROR();
        }

        return Console_ReflectionSet(active_game, Game_Metadata,
                                     Game_FieldCount, argv[1], argc - 2,
                                     argv + 2, GameTypeSetter, response_msg);
    }
    // Route dynamic reflection getters ("get main_ball.pos")
    else if (strcmp(argv[0], "get") == 0)
    {
        if (argc < 2)
        {
            snprintf(response_msg, MAX_INPUT_CHARS, "Usage: set <path>");
            return CMD_ERROR();
        }

        return Console_ReflectionGet(active_game, Game_Metadata,
                                     Game_FieldCount, argv[1], argc - 2,
                                     argv + 2, GameTypeGetter, response_msg);
    }
    // Route standard state toggles
    else if (strcmp(argv[0], "pause") == 0)
    {
        if (g.state == GAME_PAUSED)
        {
            snprintf(response_msg, MAX_INPUT_CHARS, "Game Is already paused");
            return CMD_ERROR();
        }
        g.state = GAME_PAUSED;
        snprintf(response_msg, MAX_INPUT_CHARS, "Pausing game");
        return CMD_SUCCESS();
    }
    else if (strcmp(argv[0], "resume") == 0)
    {
        if (g.state == GAME_PLAYING)
        {
            snprintf(response_msg, MAX_INPUT_CHARS, "Game Is already playing");
            return CMD_ERROR();
        }
        g.state = GAME_PLAYING;
        snprintf(response_msg, MAX_INPUT_CHARS, "Resuming game");
        return CMD_SUCCESS();
    }

    snprintf(response_msg, MAX_INPUT_CHARS, "Unrecognized command: %s",
             cmd_copy);
    return CMD_ERROR();
}

int main(void)
{
    //----------------------------------------
    // Initialization
    //----------------------------------------
    Rectangle console_rec = {5, 5, width - 10, (int)(height / 4.0f)};
    Rectangle input_box = {
        console_rec.x, console_rec.y + console_rec.height * 0.75f,
        console_rec.width, console_rec.height - console_rec.height * 0.75f};

    static const Color color_choices[9] = {
        RED, GREEN, MAGENTA, PINK, MAROON, LIME, ORANGE, GOLD, WHITE,
    };

    // Populate random bouncing balls
    for (int i = 0; i < MAX_OTHER_BALLS; i++)
    {
        Ball *b = &g.other_balls[i];
        b->radius = GetRandomValue(12, 120);

        b->pos.x = GetRandomValue(b->radius, width - b->radius);
        b->pos.y = GetRandomValue(b->radius, height - b->radius);

        b->speed.x = GetRandomValue(12, 100);
        b->speed.y = GetRandomValue(12, 100);
        b->color = color_choices[GetRandomValue(0, 8)];
    }

    InitWindow(1600, 800, "consolme - cmyreflection + raylib");
    SetTargetFPS(60);

    // Configure visual layout and theme
    ConsoleInputBoxCfg input_cfg = {
        .blink_interval = 0.5f,
        .text_color = GRAY,
        .cursor_color = WHITE,
        .text_box_color = Fade(BLACK, 0.8f),
        .box = input_box,
    };

    ConsoleConfig cfg = {
        .bounds = console_rec,
        .background = Fade(BLACK, 0.3f),
        .border = ORANGE,
        .border_width = 2.0f,
        .input_cfg = input_cfg,
        .open_key = KEY_GRAVE,
    };

    // Initialize context
    ConsoleCtx console = {0};
    console.cfg = cfg;
    console.is_open = true;
    console.on_command = GameConsole;
    console.user_data = &g;

    // Register standard commands
    // Static commands should contain elements that exist on the stack
    const char *null_args[] = {NULL};
    Console_RegisterStaticCommand(&console, "pause", null_args, 0);
    Console_RegisterStaticCommand(&console, "resume", null_args, 0);

    // Register reflection and autocompletion
    ConsoleReflectionCfg ref_cfg = {.enable_setter = true,
                                    .setter_cmd = "set",
                                    .enable_getter = true,
                                    .getter_cmd = "get"};

    Console_GenerateReflectionCompletion(&console, NULL, Game_Metadata,
                                         Game_FieldCount, ref_cfg);

    // ----------------------------------------
    // Main Game Loop
    // ----------------------------------------
    while (!WindowShouldClose())
    {
        // ----------------------------------------
        // Update
        // Process console input
        // ----------------------------------------
        Console_Update(&console);

        if (g.state == GAME_PLAYING)
        {
            float dt = GetFrameTime();

            g.main_ball.pos.x += g.main_ball.speed.x * dt;
            g.main_ball.pos.y += g.main_ball.speed.y * dt;

            if (g.main_ball.pos.x - g.main_ball.radius <= 0 ||
                g.main_ball.pos.x + g.main_ball.radius >= width)
            {
                g.main_ball.speed.x = -g.main_ball.speed.x;
            }
            if (g.main_ball.pos.y - g.main_ball.radius <= 0 ||
                g.main_ball.pos.y + g.main_ball.radius >= height)
            {
                g.main_ball.speed.y = -g.main_ball.speed.y;
            }

            for (int i = 0; i < MAX_OTHER_BALLS; i++)
            {
                Ball *b = &g.other_balls[i];
                b->pos.x += b->speed.x * dt;
                b->pos.y += b->speed.y * dt;

                if (b->pos.x - b->radius <= 0 || b->pos.x + b->radius >= width)
                {
                    b->speed.x = -b->speed.x;
                }
                if (b->pos.y - b->radius <= 0 || b->pos.y + b->radius >= height)
                {
                    b->speed.y = -b->speed.y;
                }
            }
        }

        BeginDrawing();
        ClearBackground(
            (Color){g.theme.bg_shade, g.theme.bg_shade, g.theme.bg_shade, 255});

        DrawText(TextFormat("Score: %d", g.score), 0, height * 0.8, 20, GRAY);

        DrawCircleV(g.main_ball.pos, g.main_ball.radius, g.theme.ball_color);

        for (int i = 0; i < MAX_OTHER_BALLS; i++)
        {
            Ball *b = &g.other_balls[i];
            DrawCircleV(b->pos, b->radius, b->color);
        }

        DrawText("Press ` to toggle console", 10, 570, 20, GRAY);

        Console_DrawUI(&console);
        EndDrawing();
    }

    // ----------------------------------------
    // De-Initialization
    // ----------------------------------------
    Console_Free(&console); // Free allocated memory
    CloseWindow();
};
