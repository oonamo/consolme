#ifndef CONSOLME_H
#define CONSOLME_H

#include "raylib.h"
#include <cmyreflection.h>

#ifndef MAX_LINES
#define MAX_LINES 20
#endif

#ifndef BKSP_POLL_REPEAT
#define BKSP_POLL_REPEAT 0.05f
#endif

#define CONSOLME_REQUIRE_NARGS(n, output, type)                                \
    if (argc < (n))                                                            \
    {                                                                          \
        snprintf(output, MAX_INPUT_CHARS,                                      \
                 "Error: " #type " requires %d values", n);                    \
        return CMD_ERROR();                                                    \
    }

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
    KeyboardKey open_key;
} ConsoleConfig;

typedef struct
{
    bool success;
    bool has_swatch;
    Color swatch;
} ConsoleResponse;

#define CMD_SUCCESS()                                                          \
    (ConsoleResponse) { true, false, {0} }

#define CMD_ERROR()                                                            \
    (ConsoleResponse) { false, false, {0} }

#define CMD_COLOR(color_struct)                                                \
    (ConsoleResponse) { true, true, (color_struct) }

typedef ConsoleResponse (*ConsoleCommandCallback)(const char *command,
                                                  void *user_data,
                                                  char *response_msg);

typedef struct
{
    char text[MAX_INPUT_CHARS];
    Color text_color;
    bool has_swatch;
    Color swatch;
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

    char dynamic_matches[10][MAX_INPUT_CHARS];
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
    int history_step;
    bool is_open;

    ConsoleCommandCallback on_command;
    void *user_data;

    ConsoleAutocomplete autocomplete;

    float bksp_timer;
} ConsoleCtx;

void Console_Update(ConsoleCtx *ctx);
void Console_Free(ConsoleCtx *ctx);

bool Console_RegisterStaticCommand(ConsoleCtx *ctx, const char *name,
                                   const char **args, size_t arg_count);

bool Console_RegisterDynamicCommand(ConsoleCtx *ctx, const char *name,
                                    const char **args, size_t arg_count);

int Console_Tokenize(char *buffer, char *args[], int max_args);

void Console_DrawUI(ConsoleCtx *ctx);

#ifdef CONSOLME_EXTENSION_CMYREFLECTION
#ifndef _CMYREFLECTION_H
#error                                                                         \
    "consolme: CONSOLME_EXTENSION_CMYREFLECTION is defined, but cmyreflection.h was not included before consolme.h."
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    bool enable_setter;
    const char *setter_cmd;

    bool enable_getter;
    const char *getter_cmd;
} ConsoleReflectionCfg;

typedef ConsoleResponse (*ReflectionTypeHandler)(void *target_struct,
                                                 const FieldInfo *leaf,
                                                 void *field, int argc,
                                                 char **argv,
                                                 char *response_msg);

ConsoleResponse Console_ReflectionSet(void *base_instance,
                                      const FieldInfo *base_meta,
                                      size_t base_count, const char *path,
                                      int argc, char **argv,
                                      ReflectionTypeHandler custom_handler,
                                      char *response_msg);

ConsoleResponse Console_ReflectionGet(void *base_instance,
                                      const FieldInfo *base_meta,
                                      size_t base_count, const char *path,
                                      int argc, char **argv,
                                      ReflectionTypeHandler custom_handler,
                                      char *response_msg);

void Console_GenerateReflectionCompletion(ConsoleCtx *ctx, const char *basename,
                                          const FieldInfo *metadata,
                                          size_t field_count,
                                          ConsoleReflectionCfg cfg);

#define CONSOLME_ARG_INT(idx) atoi(argv[idx])
#define CONSOLME_ARG_FLOAT(idx) (float)atof(argv[idx])
#define CONSOLME_ARG_STR(idx) (argv[idx])
#define CONSOLME_ARG_COLOR(idx) (unsigned char)CONSOLME_ARG_INT(idx)

#define DEFINE_COLOR_SETTER(func_name)                                         \
    static ConsoleResponse func_name(void *target_struct, int argc,            \
                                     char **argv, char *msg)                   \
    {                                                                          \
        Color *val = (Color *)target_struct;                                   \
        if (argc == 1)                                                         \
        {                                                                      \
            char *end;                                                         \
            unsigned long hex_val = strtoul(argv[0], &end, 16);                \
            if (*end != '\0')                                                  \
            {                                                                  \
                snprintf(msg, MAX_INPUT_CHARS,                                 \
                         "Error: Expected 0xRRGGBBAA format");                 \
                return CMD_ERROR();                                            \
            }                                                                  \
            val->r = (hex_val >> 24) & 0xFF;                                   \
            val->g = (hex_val >> 16) & 0xFF;                                   \
            val->b = (hex_val >> 8) & 0xFF;                                    \
            val->a = hex_val & 0xFF;                                           \
        }                                                                      \
        else if (argc >= 3)                                                    \
        {                                                                      \
            val->r = CONSOLME_ARG_COLOR(0);                                    \
            val->g = CONSOLME_ARG_COLOR(1);                                    \
            val->b = CONSOLME_ARG_COLOR(2);                                    \
            val->a = argc >= 4 ? CONSOLME_ARG_COLOR(3) : 255;                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            snprintf(msg, MAX_INPUT_CHARS,                                     \
                     "Error: Color requirs 1 hex argument (0xRRGGBBAA) or 3 "  \
                     "or more args (R G B A (optional)");                      \
            return CMD_ERROR();                                                \
        }                                                                      \
        snprintf(msg, MAX_INPUT_CHARS, "Set to {R:%d, G:%d, B:%d, A:%d}",      \
                 val->r, val->g, val->b, val->a);                              \
        return CMD_COLOR(*val);                                                \
    }

#define DEFINE_COLOR_GETTER(func_name)                                         \
    static ConsoleResponse func_name(void *target_struct, char *msg)           \
    {                                                                          \
        Color *val = (Color *)target_struct;                                   \
        snprintf(msg, MAX_INPUT_CHARS,                                         \
                 "{R:%d, G:%d, B:%d, A:%d} (0x%02X%02X%02X%02X)", val->r,      \
                 val->g, val->b, val->a, val->r, val->g, val->b, val->a);      \
        return CMD_COLOR(*val);                                                \
    }

#define DEFINE_FLOAT2_SETTER(func_name, struct_type, f1, f2)                   \
    static ConsoleResponse func_name(void *target_struct, int argc,            \
                                     char **argv, char *msg)                   \
    {                                                                          \
        CONSOLME_REQUIRE_NARGS(2, msg, struct_type);                           \
        struct_type *val = (struct_type *)target_struct;                       \
        val->f1 = CONSOLME_ARG_FLOAT(0);                                       \
        val->f2 = CONSOLME_ARG_FLOAT(1);                                       \
        snprintf(msg, MAX_INPUT_CHARS, #struct_type " set to [%.2f, %.2f]",    \
                 val->f1, val->f2);                                            \
        return CMD_SUCCESS();                                                  \
    }

#define DEFINE_FLOAT2_GETTER(func_name, struct_type, f1, f2)                   \
    static ConsoleResponse func_name(void *target_struct, char *msg)           \
    {                                                                          \
        struct_type *val = (struct_type *)target_struct;                       \
        snprintf(msg, MAX_INPUT_CHARS, "[%.2f, %.2f]", val->f1, val->f2);      \
        return CMD_SUCCESS();                                                  \
    }

#define DEFINE_FLOAT4_SETTER(func_name, struct_type, f1, f2, f3, f4)           \
    static ConsoleResponse func_name(void *target_struct, int argc,            \
                                     char **argv, char *msg)                   \
    {                                                                          \
        CONSOLME_REQUIRE_NARGS(2, msg, struct_type);                           \
        struct_type *val = (struct_type *)target_struct;                       \
        val->f1 = CONSOLME_ARG_FLOAT(0);                                       \
        val->f2 = CONSOLME_ARG_FLOAT(1);                                       \
        val->f3 = CONSOLME_ARG_FLOAT(2);                                       \
        val->f4 = CONSOLME_ARG_FLOAT(3);                                       \
        snprintf(msg, MAX_INPUT_CHARS,                                         \
                 #struct_type " set to [%.2f, %.2f, %.2f, %2.f]", val->f1,     \
                 val->f2, val->f3, val->f4);                                   \
        return CMD_SUCCESS();                                                  \
    }

#define DEFINE_FLOAT4_GETTER(func_name, struct_type, f1, f2)                   \
    static ConsoleResponse func_name(void *target_struct, char *msg)           \
    {                                                                          \
        struct_type *val = (struct_type *)target_struct;                       \
        snprintf(msg, MAXMAX_INPUT_CHARS, "[%.2f, %.2f, %.2f, %.2f]", val->f1, \
                 val->f2, val->f3, val->f4);                                   \
        return CMD_SUCCESS();                                                  \
    }

#endif

#endif

#ifdef CONSOLME_IMPLEMENTATION
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

char *Console_StrDup(const char *src);

char *Console_StrDup(const char *src)
{
    if (!src) return NULL;
    size_t len = strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (dst) memcpy(dst, src, len);
    return dst;
}

static void __Console_PushHistory(ConsoleCtx *ctx, const char *text,
                                  Color text_color, bool has_swatch,
                                  Color swatch)
{
    if (ctx->history_count >= MAX_LINES)
    {
        memmove(&ctx->history[0], &ctx->history[1],
                (MAX_LINES - 1) * sizeof(ConsoleLine));
        ctx->history_count = MAX_LINES - 1;
    }

    memcpy(ctx->history[ctx->history_count].text, text, MAX_INPUT_CHARS);
    ctx->history[ctx->history_count].text_color = text_color;
    ctx->history[ctx->history_count].has_swatch = has_swatch;
    ctx->history[ctx->history_count].swatch = swatch;
    ctx->history_count++;
}

static void __Console_UpdateAutocomplete(ConsoleCtx *ctx)
{
    ConsoleAutocomplete *ac = &ctx->autocomplete;
    ConsoleInputBox *box = &ctx->box;

    ac->match_count = 0;
    ac->selected_match = 0;

    // TODO: show all available commands
    if (box->buffer_len == 0) { ac->is_active = 0; }

    char *space_ptr = strchr(box->buffer, ' ');

    // Is top level command
    if (space_ptr == NULL)
    {
        ac->is_arg_completion = false;
        for (size_t i = 0; i < ac->command_count; i++)
        {
            if (strncmp(box->buffer, ac->commands[i].name, box->buffer_len) ==
                0)
            {
                ac->match_strings[ac->match_count++] = ac->commands[i].name;
                if (ac->match_count >= 10) break;
            }
        }
    }
    else
    {
        // Completing an argument
        ac->is_arg_completion = true;

        size_t cmd_len = space_ptr - box->buffer;
        size_t prefix_len = cmd_len + 1;
        strncpy(ac->base_cmd_buf, box->buffer, prefix_len);
        ac->base_cmd_buf[prefix_len] = '\0';

        const ConsoleCommandDef *active_cmd = NULL;

        for (size_t i = 0; i < ac->command_count; i++)
        {
            if (strlen(ac->commands[i].name) == cmd_len &&
                strncmp(box->buffer, ac->commands[i].name, cmd_len) == 0)
            {
                active_cmd = &ac->commands[i];
                break;
            }
        }

        if (active_cmd != NULL && active_cmd->args != NULL)
        {
            const char *arg_typed = space_ptr + 1;
            size_t arg_len = strlen(arg_typed);

            for (size_t i = 0; i < active_cmd->arg_count; i++)
            {
                const char *cand = active_cmd->args[i];
                if (strncmp(arg_typed, cand, arg_len) == 0)
                {
                    const char *dot_ptr = strchr(cand + arg_len, '.');

                    if (dot_ptr != NULL)
                    {
                        size_t display_len = (dot_ptr - cand) + 1;

                        bool is_dup = false;
                        for (size_t m = 0; m < ac->match_count; m++)
                        {
                            if (strncmp(ac->match_strings[m], cand,
                                        display_len) == 0 &&
                                ac->match_strings[m][display_len] == '\0')
                            {
                                is_dup = true;
                                break;
                            }
                        }

                        if (!is_dup)
                        {
                            strncpy(ac->dynamic_matches[ac->match_count], cand,
                                    display_len);
                            ac->dynamic_matches[ac->match_count][display_len] =
                                '\0';

                            ac->match_strings[ac->match_count] =
                                ac->dynamic_matches[ac->match_count];

                            ac->match_count++;
                        }
                    }
                    else
                    {
                        ac->match_strings[ac->match_count] =
                            active_cmd->args[i];

                        ac->match_count++;
                    }
                    if (ac->match_count >= 10) break;
                }
            }
        }
    }

    ac->is_active = (ac->match_count > 0);
}

static void __Console_ShowBlinker(ConsoleInputBox *box)
{
    box->blink_timer = 0.0f;
    box->cursor_visible = true;
}

static bool __Console_SetHistoryStep(ConsoleCtx *ctx, int target_step)
{
    ConsoleInputBox *box = &ctx->box;
    int found_count = 0;

    for (int i = (int)ctx->history_count - 1; i >= 0; i--)
    {
        if (ctx->history[i].text[0] == '>')
        {
            found_count++;
            if (found_count == target_step)
            {
                ctx->history_step = target_step;
                strncpy(box->buffer, ctx->history[i].text + 2, MAX_INPUT_CHARS);
                box->buffer_len = strlen(box->buffer);
                box->cursor_pos = box->buffer_len;

                return true;
            }
        }
    }

    return false;
}

void Console_Update(ConsoleCtx *ctx)
{
    if (IsKeyPressed(ctx->cfg.open_key))
    {
        ctx->is_open = !ctx->is_open;
        return;
    }

    if (!ctx->is_open) return;

    ctx->bksp_timer += GetFrameTime();

    ConsoleInputBox *box = &ctx->box;
    ConsoleAutocomplete *ac = &ctx->autocomplete;
    bool buffer_changed = false;
    bool cursor_changed = false;

    int key = GetCharPressed();
    while (key > 0)
    {
        if ((key >= 32) && (key <= 125) &&
            (box->buffer_len < MAX_INPUT_CHARS - 1))
        {
            memmove(&box->buffer[box->cursor_pos + 1],
                    &box->buffer[box->cursor_pos],
                    box->buffer_len - box->cursor_pos + 1);

            box->buffer[box->cursor_pos] = (char)key;
            box->cursor_pos++;
            box->buffer_len++;

            buffer_changed = true;
        }

        key = GetCharPressed();
    }

    if (((IsKeyPressedRepeat(KEY_BACKSPACE) &&
          ctx->bksp_timer > BKSP_POLL_REPEAT) ||
         IsKeyPressed(KEY_BACKSPACE)) &&
        box->cursor_pos > 0)
    {
        ctx->bksp_timer = 0;

        memmove(&box->buffer[box->cursor_pos - 1],
                &box->buffer[box->cursor_pos],
                box->buffer_len - box->cursor_pos + 1);
        box->cursor_pos--;
        box->buffer_len--;

        buffer_changed = true;
        cursor_changed = true;
    }

    if (ac->is_active)
    {
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P) &&
            ac->selected_match > 0)
            ac->selected_match--;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N) &&
            ac->selected_match < ac->match_count - 1)
            ac->selected_match++;
        if (IsKeyPressed(KEY_TAB))
        {
            const char *match = ac->match_strings[ac->selected_match];

            if (ac->is_arg_completion)
            {
                bool is_node = match[strlen(match) - 1] == '.';
                snprintf(box->buffer, MAX_INPUT_CHARS, "%s%s%s",
                         ac->base_cmd_buf, match, is_node ? "" : " ");
            }
            else { snprintf(box->buffer, MAX_INPUT_CHARS, "%s ", match); }

            box->buffer_len = strlen(box->buffer);
            box->cursor_pos = box->buffer_len;
            buffer_changed = true;
            cursor_changed = true;
        }
    }
    else
    {
        // TODO: scroll stuff later
    }

    if (IsKeyPressed(KEY_UP))
    {
        int target_step = ctx->history_step + 1;

        if (__Console_SetHistoryStep(ctx, target_step))
        {
            cursor_changed = true;
            buffer_changed = true;
        }
    }

    if (IsKeyPressed(KEY_DOWN) && ctx->history_step > 0)
    {
        ctx->history_step--;

        if (ctx->history_step == 0)
        {
            memset(box->buffer, 0, MAX_INPUT_CHARS);
            box->buffer_len = 0;
            box->cursor_pos = 0;
            buffer_changed = true;
            cursor_changed = true;
        }
        else
        {
            int target_step = ctx->history_step;
            if (__Console_SetHistoryStep(ctx, target_step))
            {
                cursor_changed = true;
                buffer_changed = true;
            }
        }
    }

    if (IsKeyPressed(KEY_LEFT) && box->cursor_pos > 0)
    {
        box->cursor_pos--;
        cursor_changed = true;
    }
    if (IsKeyPressed(KEY_RIGHT) && box->cursor_pos < box->buffer_len)
    {
        box->cursor_pos++;
        cursor_changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A))
    {
        box->cursor_pos = 0;
        cursor_changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
    {
        box->cursor_pos = box->buffer_len;
        cursor_changed = true;
    }

    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F))
    {
        size_t p = box->cursor_pos;

        while (p < box->buffer_len &&
               (box->buffer[p] == ' ' || box->buffer[p] == '.'))
        {
            p++;
        }

        while (p < box->buffer_len && box->buffer[p] != ' ' &&
               box->buffer[p] != '.')
        {
            p++;
        }

        if (p < box->buffer_len)
        {
            if (box->buffer[p] == '.') { p++; }
            else if (box->buffer[p] == ' ')
            {
                while (p < box->buffer_len && box->buffer[p] == ' ') p++;
            }
        }

        box->cursor_pos = p;
        cursor_changed = true;
    }

    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_B))
    {
        size_t p = box->cursor_pos;

        while (p > 0 &&
               (box->buffer[p - 1] == ' ' || box->buffer[p - 1] == '.'))
        {
            p--;
        }
        while (p > 0 && box->buffer[p - 1] != ' ' && box->buffer[p - 1] != '.')
        {
            p--;
        }

        box->cursor_pos = p;
        cursor_changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_W))
    {
        size_t p = box->cursor_pos;

        while (p > 0 &&
               (box->buffer[p - 1] == ' ' || box->buffer[p - 1] == '.'))
        {
            p--;
        }
        while (p > 0 && box->buffer[p - 1] != ' ' && box->buffer[p - 1] != '.')
        {
            p--;
        }
        while (p > 0 &&
               (box->buffer[p - 1] == ' ' || box->buffer[p - 1] == '.'))
        {
            p--;
        }

        size_t chars_to_del = box->cursor_pos - p;

        memmove(&box->buffer[p], &box->buffer[box->cursor_pos],
                box->buffer_len - box->cursor_pos + 1);
        box->buffer_len -= chars_to_del;

        box->cursor_pos = p;

        cursor_changed = true;
        buffer_changed = true;
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        if (box->buffer_len == 0) return;
        char response_msg[MAX_INPUT_CHARS] = {0};
        ConsoleResponse resp = CMD_SUCCESS();

        if (ctx->on_command != NULL)
        {
            resp = ctx->on_command(box->buffer, ctx->user_data, response_msg);
        }

        char history_input[MAX_INPUT_CHARS];
        snprintf(history_input, MAX_INPUT_CHARS, "> %s", box->buffer);
        __Console_PushHistory(ctx, history_input, resp.success ? RAYWHITE : RED,
                              false, BLANK);

        if (response_msg[0] != '\0')
        {
            char history_response[MAX_INPUT_CHARS];
            snprintf(history_response, MAX_INPUT_CHARS, "  %s", response_msg);
            __Console_PushHistory(ctx, history_response,
                                  resp.success ? LIGHTGRAY : RED,
                                  resp.has_swatch, resp.swatch);
        }

        memset(box->buffer, 0, MAX_INPUT_CHARS);
        box->buffer_len = 0;
        box->cursor_pos = 0;

        ac->is_active = false;
        ctx->history_step = 0;
        buffer_changed = false;
        cursor_changed = true;
    }

    if (cursor_changed) { __Console_ShowBlinker(box); }

    if (buffer_changed) { __Console_UpdateAutocomplete(ctx); }
    else
    {

        box->blink_timer += GetFrameTime();
        ctx->bksp_timer = BKSP_POLL_REPEAT;
        if (box->blink_timer >= ctx->cfg.input_cfg.blink_interval)
        {
            box->cursor_visible = !box->cursor_visible;
            box->blink_timer = 0.0f;
        }
    }
}

static ConsoleCommandDef *Console_EnsureCapacity(ConsoleAutocomplete *ac)
{
    if (ac->command_count >= ac->command_capacity)
    {
        size_t new_cap =
            (ac->command_capacity == 0) ? 16 : ac->command_capacity * 2;

        ConsoleCommandDef *new_cmds =
            realloc(ac->commands, new_cap * sizeof(ConsoleCommandDef));

        if (!new_cmds) return NULL;

        ac->commands = new_cmds;
        ac->command_capacity = new_cap;
    }

    return &ac->commands[ac->command_count];
}

bool Console_RegisterStaticCommand(ConsoleCtx *ctx, const char *name,
                                   const char **args, size_t arg_count)
{
    ConsoleCommandDef *cmd = Console_EnsureCapacity(&ctx->autocomplete);
    if (!cmd) return false;

    cmd->name = name;
    cmd->args = args;
    cmd->arg_count = arg_count;
    cmd->owns_memory = false;

    ctx->autocomplete.command_count++;
    return false;
}

bool Console_RegisterDynamicCommand(ConsoleCtx *ctx, const char *name,
                                    const char **args, size_t arg_count)
{
    ConsoleCommandDef *cmd = Console_EnsureCapacity(&ctx->autocomplete);
    if (!cmd) return false;

    cmd->name = Console_StrDup(name);
    cmd->arg_count = arg_count;
    cmd->owns_memory = true;

    if (arg_count > 0 && args != NULL)
    {
        char **dyn_args = (char **)malloc(arg_count * sizeof(char *));
        for (size_t i = 0; i < arg_count; i++)
        {
            dyn_args[i] = Console_StrDup(args[i]);
        }
        cmd->args = (const char **)dyn_args;
    }
    else { cmd->args = NULL; }

    ctx->autocomplete.command_count++;
    return true;
}

void Console_Free(ConsoleCtx *ctx)
{
    ConsoleAutocomplete *ac = &ctx->autocomplete;

    for (size_t i = 0; i < ac->command_count; i++)
    {
        ConsoleCommandDef *cmd = &ac->commands[i];

        if (cmd->owns_memory)
        {
            free((void *)cmd->name);

            if (cmd->args)
            {
                for (size_t j = 0; j < cmd->arg_count; j++)
                {
                    free((void *)cmd->args[j]);
                }
                free((void *)cmd->args);
            }
        }
    }

    free(ac->commands);
    ac->commands = NULL;
    ac->command_count = 0;
    ac->command_capacity = 0;
}

int Console_Tokenize(char *buffer, char *args[], int max_args)
{
    int argc = 0;
    bool in_quotes = false;
    char *ptr = buffer;

    while (*ptr == ' ') ptr++;

    while (*ptr != '\0' && argc < max_args)
    {
        args[argc++] = ptr;

        while (*ptr != '\0')
        {
            if (*ptr == '"') { in_quotes = !in_quotes; }
            else if (*ptr == ' ' && !in_quotes) { break; }
            ptr++;
        }

        if (*ptr != '\0')
        {
            *ptr = '\0';
            ptr++;
            while (*ptr == ' ') ptr++;
        }
    }

    return argc;
}

static void __Console_Draw_History(ConsoleCtx *ctx)
{
    ConsoleConfig *cfg = &ctx->cfg;
    int font_size = 20;
    int spacing = 2;
    int start_y = cfg->bounds.y + cfg->bounds.height -
                  cfg->input_cfg.box.height - font_size - 10;

    for (int i = ctx->history_count - 1; i >= 0; i--)
    {
        ConsoleLine *line = &ctx->history[i];
        DrawText(line->text, cfg->bounds.x + 10, start_y, font_size,
                 line->text_color);
        if (line->has_swatch)
        {
            int text_width = MeasureText(line->text, font_size);
            int padding = 10;
            int swatch_size = font_size - 4;

            int swatch_x = cfg->bounds.x + 10 + text_width + padding;
            int swatch_y = start_y + 2;

            DrawRectangle(swatch_x, swatch_y, swatch_size, swatch_size,
                          line->swatch);
            DrawRectangleLines(swatch_x, swatch_y, swatch_size, swatch_size,
                               RAYWHITE);
        }
        start_y -= (font_size + spacing);
        if (start_y < cfg->bounds.y) break; // Don't draw outside bounds
    }
}

static void __Console_Draw_TextBox(ConsoleCtx *ctx)
{
    ConsoleInputBoxCfg *cfg = &ctx->cfg.input_cfg;
    ConsoleInputBox *box = &ctx->box;

    DrawRectangleRec(cfg->box, cfg->text_box_color);

    int font_size = 20;
    DrawText(box->buffer, cfg->box.x + 5,
             cfg->box.y + (cfg->box.height - font_size) / 2, font_size,
             cfg->text_color);

    if (box->cursor_visible)
    {
        // Calculate width of string up to cursor to place the cursor correctly
        char temp[MAX_INPUT_CHARS];
        strncpy(temp, box->buffer, box->cursor_pos);
        temp[box->cursor_pos] = '\0';

        int text_width = MeasureText(temp, font_size);
        DrawRectangle(cfg->box.x + 5 + text_width,
                      cfg->box.y + (cfg->box.height - font_size) / 2, 10,
                      font_size, cfg->cursor_color);
    }
}

static void __Console_Draw_Autocomplete(ConsoleCtx *ctx)
{
    ConsoleAutocomplete *ac = &ctx->autocomplete;
    if (!ac->is_active) return;

    ConsoleConfig *cfg = &ctx->cfg;
    int font_size = 20;
    int row_height = font_size + 4;

    Rectangle popup = {cfg->input_cfg.box.x,
                       cfg->input_cfg.box.y - (ac->match_count * row_height) -
                           4,
                       250, (ac->match_count * row_height) + 4};

    DrawRectangleRec(popup, DARKGRAY);
    DrawRectangleLinesEx(popup, 1.0f, GRAY);

    for (size_t i = 0; i < ac->match_count; i++)
    {
        int y_pos = popup.y + 2 + (i * row_height);

        if (i == ac->selected_match)
        {
            DrawRectangle(popup.x + 1, y_pos, popup.width - 2, row_height,
                          GRAY);
        }

        const char *text = ac->match_strings[i];
        DrawText(text, popup.x + 5, y_pos + 2, font_size, RAYWHITE);
    }
}

static void __Console_Draw_Console(ConsoleCtx *ctx)
{
    ConsoleConfig *cfg = &ctx->cfg;
    DrawRectangleRec(cfg->bounds, cfg->background);
    DrawRectangleLinesEx(
        (Rectangle){cfg->bounds.x - cfg->border_width,
                    cfg->bounds.y - cfg->border_width,
                    cfg->bounds.width + 2 * cfg->border_width,
                    cfg->bounds.height + 2 * cfg->border_width},
        cfg->border_width, cfg->border);
}

void Console_DrawUI(ConsoleCtx *ctx)
{
    if (!ctx->is_open) return;

    __Console_Draw_Console(ctx);
    __Console_Draw_History(ctx);
    __Console_Draw_TextBox(ctx);
    __Console_Draw_Autocomplete(ctx);
}

#ifdef CONSOLME_EXTENSION_CMYREFLECTION
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char **items;
    size_t count;
    size_t capacity;
} StringList;

typedef struct
{
    char *prefix;
    const FieldInfo *fields;
    size_t field_count;
} QueueItem;

static void PushStr(StringList *list, char *str)
{
    if (list->count >= list->capacity)
    {
        list->capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->items = realloc(list->items, sizeof(char *) * list->capacity);
    }
    list->items[list->count++] = str;
}

void Console_GenerateReflectionCompletion(ConsoleCtx *ctx, const char *basename,
                                          const FieldInfo *metadata,
                                          size_t field_count,
                                          ConsoleReflectionCfg cfg)
{
    StringList results = {0};

    size_t q_capacity = 32;
    QueueItem *queue = malloc(q_capacity * sizeof(QueueItem));
    size_t q_head = 0;
    size_t q_tail = 0;

    if (basename != NULL) { queue[q_tail].prefix = Console_StrDup(basename); }
    else { queue[q_tail].prefix = NULL; }

    queue[q_tail].fields = metadata;
    queue[q_tail].field_count = field_count;
    q_tail++;

    while (q_head < q_tail)
    {
        QueueItem current = queue[q_head++];

        for (size_t i = 0; i < current.field_count; i++)
        {
            const FieldInfo *field = &current.fields[i];
            char *path = NULL;

            if (current.prefix == NULL)
            {
                size_t len = strlen(field->name) + 1;
                path = malloc(len);
                snprintf(path, len, "%s", field->name);
            }
            else
            {
                size_t len =
                    strlen(current.prefix) + 1 + strlen(field->name) + 1;
                path = malloc(len);
                snprintf(path, len, "%s.%s", current.prefix, field->name);
            }

            StructMetaData child_meta = {0};
            if (get_struct_metadata(field->type, &child_meta))
            {
                if (q_tail >= q_capacity)
                {
                    q_capacity *= 2;
                    queue = realloc(queue, q_capacity * sizeof(QueueItem));
                }

                queue[q_tail].prefix = path;
                queue[q_tail].fields = child_meta.fields;
                queue[q_tail].field_count = child_meta.count;
                q_tail++;
            }
            else { PushStr(&results, path); }
        }

        free(current.prefix);
    }

    free(queue);

    if (results.count > 0)
    {
        if (cfg.enable_setter && cfg.setter_cmd != NULL)
        {
            Console_RegisterDynamicCommand(ctx, cfg.setter_cmd,
                                           (const char **)results.items,
                                           results.count);
        }
        if (cfg.enable_getter && cfg.getter_cmd != NULL)
        {
            Console_RegisterDynamicCommand(ctx, cfg.getter_cmd,
                                           (const char **)results.items,
                                           results.count);
        }
    }

    for (size_t i = 0; i < results.count; i++) { free(results.items[i]); }

    free(results.items);
}

static void *__Console_GetTargetStruct(void *base_instance,
                                       const FieldInfo *base_meta,
                                       size_t base_count, const char *path,
                                       const FieldInfo **leaf)
{
    char clean_path[MAX_INPUT_CHARS];
    snprintf(clean_path, sizeof(clean_path), "%s", path);

    size_t len = strlen(clean_path);
    if (len > 0 && clean_path[len - 1] == '.') { clean_path[len - 1] = '\0'; }

    return resolve_field_path(base_instance, base_meta, base_count, clean_path,
                              leaf);
}

ConsoleResponse Console_ReflectionSet(void *base_instance,
                                      const FieldInfo *base_meta,
                                      size_t base_count, const char *path,
                                      int argc, char **argv,
                                      ReflectionTypeHandler custom_handler,
                                      char *response_msg)
{
    const FieldInfo *leaf = NULL;
    void *target_struct = __Console_GetTargetStruct(base_instance, base_meta,
                                                    base_count, path, &leaf);

    if (!target_struct || !leaf)
    {
        snprintf(response_msg, MAX_INPUT_CHARS,
                 "Error: Could not resolve path '%s'", path);
        return CMD_ERROR();
    }

    if (argc < 1) return CMD_ERROR();

    void *target = (char *)target_struct + leaf->offset;

    if (leaf->type == TYPE_INT)
    {
        int val = CONSOLME_ARG_INT(0);

        if (!set_field_int(target_struct, leaf, val)) return CMD_ERROR();

        snprintf(response_msg, MAX_INPUT_CHARS, "Set to %d", val);
        return CMD_SUCCESS();
    }
    else if (leaf->type == TYPE_FLOAT)
    {
        float val = CONSOLME_ARG_FLOAT(0);

        if (!set_field_float(target_struct, leaf, val))
        {
            snprintf(response_msg, MAX_INPUT_CHARS, "Could not set to %.2f",
                     val);
            return CMD_ERROR();
        };

        snprintf(response_msg, MAX_INPUT_CHARS, "Set to %.2f", val);
        return CMD_SUCCESS();
    }
    else if (leaf->type == TYPE_STR)
    {
        snprintf(response_msg, MAX_INPUT_CHARS, "Unimplmented Str");
        /// set_field_str();
        return CMD_ERROR();
    }

    if (custom_handler != NULL)
    {
        return custom_handler(target_struct, leaf,
                              (char *)target_struct + leaf->offset, argc, argv,
                              response_msg);
    }

    snprintf(response_msg, MAX_INPUT_CHARS,
             "Error: No type handler for path '%s'", path);

    return CMD_ERROR();
}

ConsoleResponse Console_ReflectionGet(void *base_instance,
                                      const FieldInfo *base_meta,
                                      size_t base_count, const char *path,
                                      int argc, char **argv,
                                      ReflectionTypeHandler custom_handler,
                                      char *response_msg)
{

    const FieldInfo *leaf = NULL;
    void *target_struct = __Console_GetTargetStruct(base_instance, base_meta,
                                                    base_count, path, &leaf);

    if (!leaf || !target_struct)
    {
        snprintf(response_msg, MAX_INPUT_CHARS,
                 "Error: Could not resolve path '%s'", path);
        return CMD_ERROR();
    }

    void *target = (char *)target_struct + leaf->offset;

    if (leaf->type == TYPE_INT)
    {
        int val = *(int *)target;
        snprintf(response_msg, MAX_INPUT_CHARS, "%s = %d", path, val);
        return CMD_SUCCESS();
    }
    else if (leaf->type == TYPE_FLOAT)
    {
        float val = *(float *)target;
        snprintf(response_msg, MAX_INPUT_CHARS, "%s = %.2f", path, val);
        return CMD_SUCCESS();
    }
    else if (leaf->type == TYPE_STR)
    {
        char *val = target;
        snprintf(response_msg, MAX_INPUT_CHARS, "%s = %s", path, val);
        return CMD_SUCCESS();
    }

    if (custom_handler != NULL)
    {
        return custom_handler(target_struct, leaf,
                              (char *)target_struct + leaf->offset, argc, argv,
                              response_msg);
    }

    snprintf(response_msg, MAX_INPUT_CHARS, "%s = %p", path, target);
    return CMD_SUCCESS();
}
#endif

#endif
