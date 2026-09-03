// clang-format off
#include "types.h"
#include "refl.generated.h"
// clang-format on

#include "consolme/render.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>

static Game g = {0};

bool PrintCommand(const char *command, void *user_data, char *error_msg)
{
    char cmd_copy[MAX_INPUT_CHARS];
    strncpy(cmd_copy, command, MAX_INPUT_CHARS);
    cmd_copy[MAX_INPUT_CHARS - 1] = '\0';

    char *argv[10];
    int argc = Console_Tokenize(cmd_copy, argv, 10);

    if (argc == 0) return true;

    if (strcmp(argv[0], "color") == 0)
    {
        if (argc < 2)
        {
            snprintf(error_msg, MAX_INPUT_CHARS,
                     "Error: 'color' requires an argument");
            return false;
        }

        if (strcmp(argv[1], "red") == 0) { g.background_color = RED; }
        else if (strcmp(argv[1], "blue") == 0) { g.background_color = BLUE; }
        else if (strcmp(argv[1], "raywhite") == 0)
        {
            g.background_color = RAYWHITE;
        }
        else
        {
            snprintf(error_msg, MAX_INPUT_CHARS,
                     "Error: color '%s' was not found", argv[1]);
            return false;
        }
    }

    return true;
}

static const char *core_args[] = {"restart", "exit"};
static const char *color_args[] = {"red", "blue", "raywhite"};

int main(void)
{
    static const int width = 1600;
    static const int height = 800;

    g.level = 1;
    g.background_color = RAYWHITE;

    Rectangle console_rec = {5, 5, width - 10, (int)(height / 4.0f)};
    Rectangle input_box = {
        console_rec.x, console_rec.y + console_rec.height * 0.75f,
        console_rec.width, console_rec.height - console_rec.height * 0.75f};

    InitWindow(1600, 800, "consolme");
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
    };

    ConsoleCtx ctx = {0};
    ctx.cfg = cfg;
    ctx.is_open = true;
    ctx.on_command = PrintCommand;

    Console_RegisterStaticCommand(&ctx, "sys", core_args, 2);
    Console_RegisterStaticCommand(&ctx, "color", color_args, 3);

    while (!WindowShouldClose())
    {
        Console_Update(&ctx);
        BeginDrawing();
        ClearBackground(g.background_color);
        Console_DrawUI(&ctx);
        EndDrawing();
    }

    Console_Free(&ctx);

    CloseWindow();
};
