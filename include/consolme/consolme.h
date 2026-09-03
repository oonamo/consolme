#ifndef _consolme_H
#define _consolme_H

#include "raylib.h"
#include <cmyreflection.h>

#ifndef MAX_LINES
#define MAX_LINES 20
#endif

typedef struct
{
    Rectangle box;
    float blink_interval;

    Color text_color;
    Color cursor_color;
    Color text_box_color;
} ConsoleInputBoxCfg;

#define MAX_INPUT_CHARS 256
typedef struct
{
    float blink_timer;
    size_t cursor_pos;
    bool cursor_visible;

    char buffer[MAX_INPUT_CHARS];
    size_t buffer_len;
} ConsoleInputBox;

typedef struct
{
    Rectangle bounds;

    float border_width;
    Color background;
    Color border;

    ConsoleInputBoxCfg input_cfg;
} ConsoleConfig;

typedef bool (*ConsoleCommandCallback)(const char *command, void *user_data,
                                       char *out_err_msg);

typedef struct
{
    char text[MAX_INPUT_CHARS];
    Color color;
} ConsoleLine;

typedef struct
{
    const char *name;
    const char **args;
    size_t arg_count;
    bool owns_memory;
} ConsoleCommandDef;

typedef struct
{
    ConsoleCommandDef *commands;
    size_t command_count;
    size_t command_capacity;

    const char *match_strings[10];
    size_t match_count;
    size_t selected_match;
    bool is_active;

    bool is_arg_completion;
    char base_cmd_buf[64];
} ConsoleAutocomplete;

typedef struct
{
    ConsoleConfig cfg;
    ConsoleInputBox box;

    ConsoleLine history[MAX_LINES];
    size_t history_count;
    bool is_open;

    ConsoleCommandCallback on_command;
    void *user_data;

    ConsoleAutocomplete autocomplete;
} ConsoleCtx;

void Console_Update(ConsoleCtx *ctx);
void Console_Free(ConsoleCtx *ctx);

bool Console_RegisterStaticCommand(ConsoleCtx *ctx, const char *name,
                                   const char **args, size_t arg_count);

bool Console_RegisterDynamicCommand(ConsoleCtx *ctx, const char *name,
                                    const char **args, size_t arg_count);

int Console_Tokenize(char *buffer, char *args[], int max_args);

#endif // _consolme_H
