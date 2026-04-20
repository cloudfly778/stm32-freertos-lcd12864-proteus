#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include "FreeRTOS.h"

#define KEYPAD_ROWS              4U
#define KEYPAD_COLS              4U
#define KEYPAD_SCAN_PERIOD_MS    10U
#define KEYPAD_DEBOUNCE_COUNT    3U
#define KEYPAD_REPEAT_START_MS   500U
#define KEYPAD_REPEAT_NEXT_MS    200U
#define KEYPAD_COL_PRESSED       0U

typedef enum {
    KEY_NONE = 0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_STAR,
    KEY_0,
    KEY_HASH,
    KEY_BACKSPACE,
    KEY_CASE_TOGGLE,
    KEY_ENTER
} KeyCode;

typedef enum {
    KEY_EVENT_SHORT = 0,
    KEY_EVENT_REPEAT
} KeyEventType;

typedef struct {
    KeyCode key;
    KeyEventType type;
    TickType_t tick;
} KeyEvent;

void Keypad_Task(void *argument);

/*
 * 由具体板级代码重写：
 * - Keypad_SetRow: 选中行并拉低（active=1）或释放（active=0）
 * - Keypad_ReadCol: 读取列电平（按下时返回0）
 */
void Keypad_SetRow(uint8_t row, uint8_t active);
uint8_t Keypad_ReadCol(uint8_t col);

#endif
