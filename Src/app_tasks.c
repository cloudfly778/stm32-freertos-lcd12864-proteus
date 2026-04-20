#include "app_tasks.h"
#include "keypad.h"
#include "lcd12864.h"
#include "t9_input.h"
#include "task.h"

AppQueues g_app_queues = {0};

BaseType_t AppTasks_Create(void)
{
    g_app_queues.key_queue = xQueueCreate(16U, sizeof(KeyEvent));
    g_app_queues.lcd_queue = xQueueCreate(1U, sizeof(DisplayFrame));
    if ((g_app_queues.key_queue == NULL) || (g_app_queues.lcd_queue == NULL)) {
        return pdFAIL;
    }

    if (xTaskCreate(Keypad_Task, "KeypadTask", 256U, NULL, tskIDLE_PRIORITY + 3U, NULL) != pdPASS) {
        return pdFAIL;
    }
    if (xTaskCreate(Input_Task, "InputTask", 512U, NULL, tskIDLE_PRIORITY + 2U, NULL) != pdPASS) {
        return pdFAIL;
    }
    if (xTaskCreate(Lcd_Task, "LcdTask", 384U, NULL, tskIDLE_PRIORITY + 1U, NULL) != pdPASS) {
        return pdFAIL;
    }
    return pdPASS;
}

void Input_Task(void *argument)
{
    (void)argument;

    T9Context ctx;
    DisplayFrame frame;
    KeyEvent event;
    const KeyEvent init_event = {.key = KEY_NONE, .type = KEY_EVENT_SHORT, .tick = 0U};

    T9_Init(&ctx);
    T9_ProcessEvent(&ctx, &init_event, xTaskGetTickCount(), &frame);
    (void)xQueueOverwrite(g_app_queues.lcd_queue, &frame);

    while (1) {
        if (xQueueReceive(g_app_queues.key_queue, &event, pdMS_TO_TICKS(50U)) == pdTRUE) {
            T9_ProcessEvent(&ctx, &event, xTaskGetTickCount(), &frame);
            (void)xQueueOverwrite(g_app_queues.lcd_queue, &frame);
        } else {
            T9_ProcessTimeout(&ctx, xTaskGetTickCount(), &frame);
            (void)xQueueOverwrite(g_app_queues.lcd_queue, &frame);
        }
    }
}

void Lcd_Task(void *argument)
{
    (void)argument;
    DisplayFrame frame;
    LCD12864_Clear();

    while (1) {
        if (xQueueReceive(g_app_queues.lcd_queue, &frame, portMAX_DELAY) == pdTRUE) {
            LCD12864_RenderFrame(&frame);
        }
    }
}
