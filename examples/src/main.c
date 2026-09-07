// clang-format off
#include "types.h"
#include "refl.generated.h"
// clang-format on

#define CONSOLME_IMPLEMENTATION
#define CONSOLME_EXTENSION_CMYREFLECTION
#include <consolme.h>

#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

static const int width = 1600;
static const int height = 800;

Game g = {.ball = {.pos = {width / 2.0f, height / 2.0f},
                   .speed = {200.0f, 150.0f},
                   .radius = 40.0f},
          .theme = {
              30,
              .ball_color = GRAY,
          }};

DEFINE_COLOR_SETTER(Set_Color);
DEFINE_FLOAT2_SETTER(Set_Vector2, Vector2, x, y);
DEFINE_FLOAT4_SETTER(Set_Rectangle, Rectangle, x, y, width, height);

bool GameTypeHandler(void *target_struct, const FieldInfo *leaf,
                     void *field_ptr, int argc, char **argv, char *response_msg)
{

    switch (leaf->type)
    {
    case TYPE_CONSTSTR:
        break;
    case TYPE_FLOAT:
        break;
    case TYPE_GAMESTATE:
        break;
    case TYPE_INT:
        break;
    case TYPE_STR:
        break;
    case TYPE_STRUCT_BALL:
        break;
    case TYPE_STRUCT_COLOR:
        return Set_Color(field_ptr, argc, argv, response_msg);
    case TYPE_STRUCT_GAME:
        break;
    case TYPE_STRUCT_THEME:
        break;
    case TYPE_STRUCT_VECTOR2:
        return Set_Vector2(field_ptr, argc, argv, response_msg);
    case TYPE_UNKNOWN:
        break;
    case TYPE_UNSIGNEDCHAR:
        break;
    }
    snprintf(response_msg, MAX_INPUT_CHARS, "Type was not implmeneted!");
    return false;
}

bool GameConsole(const char *command, void *user_data, char *response_msg)
{
    Game *active_game = (Game *)user_data;
    char cmd_copy[MAX_INPUT_CHARS];
    strncpy(cmd_copy, command, MAX_INPUT_CHARS);
    cmd_copy[MAX_INPUT_CHARS - 1] = '\0';

    char *argv[10];
    int argc = Console_Tokenize(cmd_copy, argv, 10);
    if (argc == 0) return true;

    if (strcmp(argv[0], "set") == 0)
    {
        if (argc < 3)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Usage: set <path> <value>");
            return false;
        }

        return Console_ReflectionSet(active_game, Game_Metadata,
                                     Game_FieldCount, argv[1], argc - 2,
                                     argv + 2, GameTypeHandler, response_msg);
    }
    else if (strcmp(argv[0], "get") == 0) {}

    return true;
}

int main(void)
{

    Rectangle console_rec = {5, 5, width - 10, (int)(height / 4.0f)};
    Rectangle input_box = {
        console_rec.x, console_rec.y + console_rec.height * 0.75f,
        console_rec.width, console_rec.height - console_rec.height * 0.75f};

    InitWindow(1600, 800, "consolme");
    SetTargetFPS(60);

    ConsoleInputBoxCfg input_cfg = {
        .blink_interval = 500,
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

    ConsoleCtx console = {0};
    console.cfg = cfg;
    console.is_open = true;
    console.on_command = GameConsole;
    console.user_data = &g;

    const char *stop_args[] = {NULL};
    Console_RegisterStaticCommand(&console, "stop", stop_args, 0);

    ConsoleReflectionCfg ref_cfg = {.enable_setter = true,
                                    .setter_cmd = "set",
                                    .enable_getter = true,
                                    .getter_cmd = "get"};

    Console_GenerateReflectionCompletion(&console, NULL, Game_Metadata,
                                         Game_FieldCount, ref_cfg);

    while (!WindowShouldClose())
    {
        Console_Update(&console);

        if (!console.is_open)
        {
            float dt = GetFrameTime();

            g.ball.pos.x += g.ball.speed.x * dt;
            g.ball.pos.y += g.ball.speed.y * dt;

            if (g.ball.pos.x - g.ball.radius <= 0 ||
                g.ball.pos.x + g.ball.radius >= width)
            {
                g.ball.speed.x = -g.ball.speed.x;
            }
            if (g.ball.pos.y - g.ball.radius <= 0 ||
                g.ball.pos.y + g.ball.radius >= height)
            {
                g.ball.speed.y = -g.ball.speed.y;
            }
        }

        BeginDrawing();
        ClearBackground(
            (Color){g.theme.bg_shade, g.theme.bg_shade, g.theme.bg_shade, 255});

        DrawText(TextFormat("Score: %d", g.score), 0, height * 0.8, 20, GRAY);

        DrawCircleV(g.ball.pos, g.ball.radius, g.theme.ball_color);

        DrawText("Press ` to toggle console", 10, 570, 20, GRAY);

        Console_DrawUI(&console);
        EndDrawing();
    }

    Console_Free(&console);

    CloseWindow();
};
