#include "FreeRTOS.h"
#include "task.h"
#include "app_tasks.h"
#include "lcd12864.h"

/*
 * 注意：
 * 1) 将本文件并入你现有 STM32F103 工程后，使用你的 HAL/SystemClock/GPIO 初始化实现替换下述弱符号。
 * 2) Keypad/LCD 的实际 GPIO 读写函数在 board 层重写 keypad.c 与 lcd12864.c 的 weak 函数即可。
 */
void Board_Init(void);
void Error_Handler(void);

int main(void)
{
    Board_Init();
    LCD12864_Init();

    if (AppTasks_Create() != pdPASS) {
        Error_Handler();
    }

    vTaskStartScheduler();

    while (1) {
    }
}

__attribute__((weak)) void Board_Init(void)
{
}

__attribute__((weak)) void Error_Handler(void)
{
    taskDISABLE_INTERRUPTS();
    while (1) {
    }
}
