#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "keypad.h"
#include "lcd12864.h"

typedef struct {
    QueueHandle_t key_queue;
    QueueHandle_t lcd_queue;
} AppQueues;

BaseType_t AppTasks_Create(void);
void Input_Task(void *argument);
void Lcd_Task(void *argument);

#endif
