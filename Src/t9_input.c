#include "t9_input.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *upper;
    const char *lower;
} T9KeyMap;

static const T9KeyMap s_map[10] = {
    {"0", "0"},
    {"1", "1"},
    {"ABC", "abc"},
    {"DEF", "def"},
    {"GHI", "ghi"},
    {"JKL", "jkl"},
    {"MNO", "mno"},
    {"PQRS", "pqrs"},
    {"TUV", "tuv"},
    {"WXYZ", "wxyz"}
};

static const char *mode_to_text(InputMode mode)
{
    switch (mode) {
        case INPUT_MODE_UPPER: return "ABC";
        case INPUT_MODE_LOWER: return "abc";
        default: return "123";
    }
}

static uint8_t key_to_digit(KeyCode key, uint8_t *digit)
{
    switch (key) {
        case KEY_0: *digit = 0U; return 1U;
        case KEY_1: *digit = 1U; return 1U;
        case KEY_2: *digit = 2U; return 1U;
        case KEY_3: *digit = 3U; return 1U;
        case KEY_4: *digit = 4U; return 1U;
        case KEY_5: *digit = 5U; return 1U;
        case KEY_6: *digit = 6U; return 1U;
        case KEY_7: *digit = 7U; return 1U;
        case KEY_8: *digit = 8U; return 1U;
        case KEY_9: *digit = 9U; return 1U;
        default: return 0U;
    }
}

static void append_char(T9Context *ctx, char ch)
{
    if (ctx->len >= T9_TEXT_BUFFER_SIZE) {
        memmove(&ctx->text[0], &ctx->text[1], T9_TEXT_BUFFER_SIZE - 1U);
        ctx->len = T9_TEXT_BUFFER_SIZE - 1U;
    }
    ctx->text[ctx->len++] = ch;
    ctx->text[ctx->len] = '\0';
}

static void append_pending_char(const T9Context *ctx, char *out, size_t out_size)
{
    if ((ctx->pending_key == KEY_NONE) || (out_size < 2U)) {
        out[0] = '\0';
        return;
    }

    uint8_t digit = 0U;
    if (key_to_digit(ctx->pending_key, &digit) == 0U) {
        out[0] = '\0';
        return;
    }

    const char *candidate = (ctx->mode == INPUT_MODE_UPPER) ? s_map[digit].upper : s_map[digit].lower;
    const size_t len = strlen(candidate);
    if (len == 0U) {
        out[0] = '\0';
        return;
    }
    out[0] = candidate[ctx->pending_index % len];
    out[1] = '\0';
}

static void commit_pending(T9Context *ctx)
{
    if (ctx->pending_key == KEY_NONE) {
        return;
    }

    uint8_t digit = 0U;
    if (key_to_digit(ctx->pending_key, &digit) == 0U) {
        ctx->pending_key = KEY_NONE;
        return;
    }

    const char *candidate = (ctx->mode == INPUT_MODE_UPPER) ? s_map[digit].upper : s_map[digit].lower;
    const size_t len = strlen(candidate);
    if (len > 0U) {
        append_char(ctx, candidate[ctx->pending_index % len]);
    }
    ctx->pending_key = KEY_NONE;
    ctx->pending_index = 0U;
    ctx->pending_tick = 0U;
}

static void render_frame(const T9Context *ctx, DisplayFrame *out_frame)
{
    char display[T9_TEXT_BUFFER_SIZE + 2U];
    char pending[2];
    append_pending_char(ctx, pending, sizeof(pending));

    (void)snprintf(display, sizeof(display), "%s%s", ctx->text, pending);

    (void)snprintf(out_frame->status, sizeof(out_frame->status), "T9 %-3s Len:%03u",
                   mode_to_text(ctx->mode), (unsigned)ctx->len);

    char lines[64][LCD_COLS + 1U];
    memset(lines, ' ', sizeof(lines));
    for (size_t i = 0U; i < 64U; ++i) {
        lines[i][LCD_COLS] = '\0';
    }

    uint8_t row = 0U;
    uint8_t col = 0U;
    for (size_t i = 0U; display[i] != '\0'; ++i) {
        if (display[i] == '\n') {
            row++;
            col = 0U;
            continue;
        }
        if (col >= LCD_COLS) {
            row++;
            col = 0U;
        }
        if (row >= 64U) {
            break;
        }
        lines[row][col++] = display[i];
    }

    if (col >= LCD_COLS) {
        row++;
        col = 0U;
    }
    if (row >= 64U) {
        row = 63U;
        col = LCD_COLS - 1U;
    }

    uint8_t start = (row >= LCD_INPUT_ROWS) ? (uint8_t)(row - (LCD_INPUT_ROWS - 1U)) : 0U;
    for (uint8_t i = 0U; i < LCD_INPUT_ROWS; ++i) {
        const uint8_t src = start + i;
        if (src < 64U) {
            memcpy(out_frame->lines[i], lines[src], LCD_COLS + 1U);
        } else {
            memset(out_frame->lines[i], ' ', LCD_COLS);
            out_frame->lines[i][LCD_COLS] = '\0';
        }
    }

    out_frame->cursor_line = (uint8_t)(row - start);
    if (out_frame->cursor_line >= LCD_INPUT_ROWS) {
        out_frame->cursor_line = LCD_INPUT_ROWS - 1U;
    }
    out_frame->cursor_col = (col < LCD_COLS) ? col : (LCD_COLS - 1U);
}

void T9_Init(T9Context *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->mode = INPUT_MODE_UPPER;
}

void T9_ProcessEvent(T9Context *ctx, const KeyEvent *event, TickType_t now, DisplayFrame *out_frame)
{
    uint8_t digit = 0U;

    if (event->key == KEY_BACKSPACE) {
        if (ctx->pending_key != KEY_NONE) {
            ctx->pending_key = KEY_NONE;
            ctx->pending_index = 0U;
        } else if (ctx->len > 0U) {
            ctx->len--;
            ctx->text[ctx->len] = '\0';
        }
        render_frame(ctx, out_frame);
        return;
    }

    if (event->key == KEY_CASE_TOGGLE) {
        commit_pending(ctx);
        if (ctx->mode == INPUT_MODE_UPPER) {
            ctx->mode = INPUT_MODE_LOWER;
        } else if (ctx->mode == INPUT_MODE_LOWER) {
            ctx->mode = INPUT_MODE_NUMERIC;
        } else {
            ctx->mode = INPUT_MODE_UPPER;
        }
        render_frame(ctx, out_frame);
        return;
    }

    if (event->key == KEY_ENTER) {
        commit_pending(ctx);
        append_char(ctx, '\n');
        render_frame(ctx, out_frame);
        return;
    }

    if (event->key == KEY_STAR) {
        commit_pending(ctx);
        append_char(ctx, '*');
        render_frame(ctx, out_frame);
        return;
    }

    if (event->key == KEY_HASH) {
        commit_pending(ctx);
        append_char(ctx, '#');
        render_frame(ctx, out_frame);
        return;
    }

    if (key_to_digit(event->key, &digit) == 0U) {
        render_frame(ctx, out_frame);
        return;
    }

    if (ctx->mode == INPUT_MODE_NUMERIC) {
        commit_pending(ctx);
        append_char(ctx, (char)('0' + digit));
        render_frame(ctx, out_frame);
        return;
    }

    if (digit == 0U) {
        commit_pending(ctx);
        append_char(ctx, ' ');
        render_frame(ctx, out_frame);
        return;
    }

    if (digit == 1U) {
        commit_pending(ctx);
        append_char(ctx, '1');
        render_frame(ctx, out_frame);
        return;
    }

    if ((ctx->pending_key == event->key) &&
        ((now - ctx->pending_tick) <= pdMS_TO_TICKS(T9_MULTI_TAP_TIMEOUT_MS))) {
        const char *candidate = (ctx->mode == INPUT_MODE_UPPER) ? s_map[digit].upper : s_map[digit].lower;
        const size_t len = strlen(candidate);
        if (len > 0U) {
            ctx->pending_index = (uint8_t)((ctx->pending_index + 1U) % len);
            ctx->pending_tick = now;
        }
    } else {
        commit_pending(ctx);
        ctx->pending_key = event->key;
        ctx->pending_index = 0U;
        ctx->pending_tick = now;
    }

    render_frame(ctx, out_frame);
}

void T9_ProcessTimeout(T9Context *ctx, TickType_t now, DisplayFrame *out_frame)
{
    if ((ctx->pending_key != KEY_NONE) &&
        ((now - ctx->pending_tick) > pdMS_TO_TICKS(T9_MULTI_TAP_TIMEOUT_MS))) {
        commit_pending(ctx);
        render_frame(ctx, out_frame);
    }
}
