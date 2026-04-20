#ifndef T9_INPUT_H
#define T9_INPUT_H

#include <stddef.h>
#include "FreeRTOS.h"
#include "keypad.h"
#include "lcd12864.h"

#define T9_TEXT_BUFFER_SIZE 192U
#define T9_MULTI_TAP_TIMEOUT_MS 800U

typedef enum {
    INPUT_MODE_UPPER = 0,
    INPUT_MODE_LOWER,
    INPUT_MODE_NUMERIC
} InputMode;

typedef struct {
    char text[T9_TEXT_BUFFER_SIZE + 1U];
    size_t len;
    KeyCode pending_key;
    uint8_t pending_index;
    TickType_t pending_tick;
    InputMode mode;
} T9Context;

void T9_Init(T9Context *ctx);
void T9_ProcessEvent(T9Context *ctx, const KeyEvent *event, TickType_t now, DisplayFrame *out_frame);
void T9_ProcessTimeout(T9Context *ctx, TickType_t now, DisplayFrame *out_frame);

#endif
