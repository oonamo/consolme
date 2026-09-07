// clang-format off
#include "types.h"
#include "refl.generated.h"
// clang-format on

#include <consolme/extensions/cmyreflection.h>
#include <consolme/render.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

static Game g = {0};

bool PrintCommand(const char *command, void *user_data, char *response_msg)
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
            snprintf(response_msg, MAX_INPUT_CHARS,
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
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: color '%s' was not found", argv[1]);
            return false;
        }
    }
    else if (strcmp(argv[0], "set") == 0)
    {
        if (argc < 3)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: set requires a path and a value");
            return false;
        }

        const char *path = argv[1];
        const char *value = argv[2];

        bool had_success = false;

        const FieldInfo *leaf_field = NULL;

        void *target_ptr = resolve_field_path(
            &g, Game_Metadata, Game_FieldCount, path, &leaf_field);

        if (!target_ptr || !leaf_field)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: Could not resolve path `%s`", path);
            return false;
        }

        switch (leaf_field->type)
        {
        case TYPE_COLOR:
            break;
        case TYPE_FLOAT:
            *(float *)target_ptr = atof(value);
            had_success = true;
        case TYPE_INT:
            *(int *)target_ptr = atoi(value);
            had_success = true;
            break;
        case TYPE_STR:
            target_ptr = (char *)value;
            had_success = true;
            break;
        case TYPE_STRUCT_GAME:
            break;
        case TYPE_STRUCT_PLAYER:
            break;
        case TYPE_UNKNOWN:
            break;
        }

        if (!had_success)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: Could not set path `%s` to '%s'", path, value);
        }

        return had_success;
    }
    else if (strcmp("get", argv[0]) == 0)
    {
        if (argc != 2)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: get requires just a path");
            return false;
        }

        const char *path = argv[1];

        bool had_success = false;

        const FieldInfo *leaf_field = NULL;

        void *target_ptr = resolve_field_path(
            &g, Game_Metadata, Game_FieldCount, path, &leaf_field);

        if (!target_ptr || !leaf_field)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: Could not resolve path `%s`", path);
            return false;
        }

        switch (leaf_field->type)
        {
        case TYPE_COLOR:
            break;
        case TYPE_FLOAT:
            snprintf(response_msg, MAX_INPUT_CHARS, "%s = %f", path,
                     *(float *)target_ptr);
            had_success = true;
        case TYPE_INT:
            had_success = true;
            snprintf(response_msg, MAX_INPUT_CHARS, "%s = %d", path,
                     *(int *)target_ptr);
            break;
        case TYPE_STR:
            snprintf(response_msg, MAX_INPUT_CHARS, "%s = \"%s\"", path,
                     (char *)target_ptr);
            had_success = true;
            break;
        case TYPE_STRUCT_GAME:
            break;
        case TYPE_STRUCT_PLAYER:
            break;
        case TYPE_UNKNOWN:
            break;
        }

        if (!had_success)
        {
            snprintf(response_msg, MAX_INPUT_CHARS,
                     "Error: Could not get path `%s`", path);
        }
    }
    else
    {
        snprintf(response_msg, MAX_INPUT_CHARS, "Error: Invalid command '%s'",
                 argv[0]);
        return false;
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

    ConsoleReflectionCfg ref_cfg = {.enable_setter = true,
                                    .setter_cmd = "set",
                                    .enable_getter = true,
                                    .getter_cmd = "get"};

    Console_GenerateReflectionCompletion(&ctx, NULL, Game_Metadata,
                                         Game_FieldCount, ref_cfg);

    while (!WindowShouldClose())
    {
        Console_Update(&ctx);
        BeginDrawing();
        ClearBackground(g.background_color);
        Console_DrawUI(&ctx);

        DrawText(TextFormat("Level: %d", g.level), 0, height * 0.8, 20, BLACK);
        EndDrawing();
    }

    Console_Free(&ctx);

    CloseWindow();
};
