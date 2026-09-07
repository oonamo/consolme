#include "consoleme_internal.h"
#include "consolme/render.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

static void __Console_PushHistory(ConsoleCtx *ctx, const char *text,
                                  Color color)
{
    if (ctx->history_count >= MAX_LINES)
    {
        memmove(&ctx->history[0], &ctx->history[1],
                (MAX_LINES - 1) * sizeof(ConsoleLine));
        ctx->history_count = MAX_LINES - 1;
    }

    memcpy(ctx->history[ctx->history_count].text, text, MAX_INPUT_CHARS);
    ctx->history[ctx->history_count].color = color;
    ctx->history_count++;
}

static void __Console_UpdateAutocomplete(ConsoleCtx *ctx)
{
    ConsoleAutocomplete *ac = &ctx->autocomplete;
    ConsoleInputBox *box = &ctx->box;

    ac->match_count = 0;
    ac->selected_match = 0;

    // TODO: show all available commands
    if (box->buffer_len == 0)
    {
        ac->is_active = 0;
        return;
    }

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

void Console_Update(ConsoleCtx *ctx)
{
    if (!ctx->is_open) return;

    ConsoleInputBox *box = &ctx->box;
    ConsoleAutocomplete *ac = &ctx->autocomplete;
    bool buffer_changed = false;

    box->blink_timer += GetFrameTime();
    if (box->blink_timer >= ctx->cfg.input_cfg.blink_interval)
    {
        box->cursor_visible = !box->cursor_visible;
        box->blink_timer = 0.0f;
    }

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

    if (IsKeyPressed(KEY_BACKSPACE) && box->cursor_pos > 0)
    {
        memmove(&box->buffer[box->cursor_pos - 1],
                &box->buffer[box->cursor_pos],
                box->buffer_len - box->cursor_pos + 1);
        box->cursor_pos--;
        box->buffer_len--;

        buffer_changed = true;
    }

    if (ac->is_active)
    {
        if (IsKeyPressed(KEY_UP) && ac->selected_match > 0)
            ac->selected_match--;
        if (IsKeyPressed(KEY_DOWN) && ac->selected_match < ac->match_count - 1)
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
        }
    }
    else
    {
        if (IsKeyPressed(KEY_LEFT) && box->cursor_pos > 0)
        {
            box->cursor_pos--;
        }
        if (IsKeyPressed(KEY_RIGHT) && box->cursor_pos < box->buffer_len)
        {
            box->cursor_pos++;
        }
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        char response_msg[MAX_INPUT_CHARS] = {0};
        bool success = true;

        if (ctx->on_command != NULL)
        {
            success =
                ctx->on_command(box->buffer, ctx->user_data, response_msg);
        }

        __Console_PushHistory(ctx, box->buffer, success ? RAYWHITE : RED);

        if (response_msg[0] != '\0')
        {
            __Console_PushHistory(ctx, response_msg, success ? LIGHTGRAY : RED);
        }

        memset(box->buffer, 0, MAX_INPUT_CHARS);
        box->buffer_len = 0;
        box->cursor_pos = 0;
        buffer_changed = true;
    }

    if (buffer_changed) __Console_UpdateAutocomplete(ctx);
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
