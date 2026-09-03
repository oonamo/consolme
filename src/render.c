#include "consolme/render.h"

static void __Console_Draw_History(ConsoleCtx *ctx)
{
    ConsoleConfig *cfg = &ctx->cfg;
    int font_size = 20;
    int spacing = 2;
    int start_y = cfg->bounds.y + cfg->bounds.height -
                  cfg->input_cfg.box.height - font_size - 10;

    // Draw history from bottom to top
    for (int i = ctx->history_count - 1; i >= 0; i--)
    {
        DrawText(ctx->history[i].text, cfg->bounds.x + 10, start_y, font_size,
                 ctx->history[i].color);
        start_y -= (font_size + spacing);
        if (start_y < cfg->bounds.y) break; // Don't draw outside bounds
    }
}

static void __Console_Draw_TextBox(ConsoleCtx *ctx)
{
    ConsoleInputBoxCfg *cfg = &ctx->cfg.input_cfg;
    ConsoleInputBox *box = &ctx->box;

    DrawRectangleRec(cfg->box, cfg->text_box_color);

    // Draw Text
    int font_size = 20;
    DrawText(box->buffer, cfg->box.x + 5,
             cfg->box.y + (cfg->box.height - font_size) / 2, font_size,
             cfg->text_color);

    // Draw Cursor
    if (box->cursor_visible)
    {
        // Calculate width of string up to cursor to place the cursor correctly
        char temp[MAX_INPUT_CHARS];
        strncpy(temp, box->buffer, box->cursor_pos);
        temp[box->cursor_pos] = '\0';

        int text_width = MeasureText(temp, font_size);
        DrawRectangle(cfg->box.x + 5 + text_width, cfg->box.y + 4, 10,
                      cfg->box.height - 8, cfg->cursor_color);
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
