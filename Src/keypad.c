#include "keypad.h"
#include "app_tasks.h"
#include "queue.h"
#include "task.h"

extern AppQueues g_app_queues;

static const KeyCode s_key_map[KEYPAD_ROWS][KEYPAD_COLS] = {
    {KEY_1, KEY_2, KEY_3, KEY_BACKSPACE},
    {KEY_4, KEY_5, KEY_6, KEY_CASE_TOGGLE},
    {KEY_7, KEY_8, KEY_9, KEY_ENTER},
    {KEY_STAR, KEY_0, KEY_HASH, KEY_NONE}
};

static KeyCode keypad_scan_once(void)
{
    for (uint8_t r = 0U; r < KEYPAD_ROWS; ++r) {
        for (uint8_t i = 0U; i < KEYPAD_ROWS; ++i) {
            Keypad_SetRow(i, 0U);
        }

        Keypad_SetRow(r, 1U);

        for (uint8_t c = 0U; c < KEYPAD_COLS; ++c) {
            if (Keypad_ReadCol(c) == KEYPAD_COL_PRESSED) {
                Keypad_SetRow(r, 0U);
                return s_key_map[r][c];
            }
        }
        Keypad_SetRow(r, 0U);
    }
    return KEY_NONE;
}

void Keypad_Task(void *argument)
{
    (void)argument;

    KeyCode stable_key = KEY_NONE;
    KeyCode candidate = KEY_NONE;
    uint8_t stable_count = 0U;
    TickType_t press_tick = 0U;
    TickType_t last_repeat_tick = 0U;

    while (1) {
        const KeyCode sampled = keypad_scan_once();

        if (sampled == candidate) {
            if (stable_count < 0xFFU) {
                ++stable_count;
            }
        } else {
            candidate = sampled;
            stable_count = 1U;
        }

        if ((stable_count >= KEYPAD_DEBOUNCE_COUNT) && (candidate != stable_key)) {
            stable_key = candidate;
            if (stable_key != KEY_NONE) {
                KeyEvent evt = {
                    .key = stable_key,
                    .type = KEY_EVENT_SHORT,
                    .tick = xTaskGetTickCount()
                };
                (void)xQueueSend(g_app_queues.key_queue, &evt, 0U);
                press_tick = evt.tick;
                last_repeat_tick = evt.tick;
            }
        }

        if (stable_key != KEY_NONE) {
            const TickType_t now = xTaskGetTickCount();
            const TickType_t press_elapsed = now - press_tick;
            const TickType_t repeat_elapsed = now - last_repeat_tick;
            if ((press_elapsed >= pdMS_TO_TICKS(KEYPAD_REPEAT_START_MS)) &&
                (repeat_elapsed >= pdMS_TO_TICKS(KEYPAD_REPEAT_NEXT_MS))) {
                KeyEvent evt = {
                    .key = stable_key,
                    .type = KEY_EVENT_REPEAT,
                    .tick = now
                };
                (void)xQueueSend(g_app_queues.key_queue, &evt, 0U);
                last_repeat_tick = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(KEYPAD_SCAN_PERIOD_MS));
    }
}

__attribute__((weak)) void Keypad_SetRow(uint8_t row, uint8_t active)
{
    (void)row;
    (void)active;
}

__attribute__((weak)) uint8_t Keypad_ReadCol(uint8_t col)
{
    (void)col;
    /* 弱符号默认返回未按下（active-low 键值中 1=release, 0=pressed） */
    return 1U;
}
