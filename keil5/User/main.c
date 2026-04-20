#include <stdint.h>
#include "cmsis_os2.h"

static void lcd12864_init(void);
static void lcd12864_clear(void);
static void lcd12864_show_string(uint8_t row, uint8_t col, const char *text);
static void display_task(void *argument);

int main(void)
{
    SystemCoreClockUpdate();

    osKernelInitialize();
    lcd12864_init();
    lcd12864_clear();

    const osThreadAttr_t display_task_attr = {
        .name = "displayTask",
        .priority = osPriorityNormal,
        .stack_size = 512U
    };

    (void)osThreadNew(display_task, NULL, &display_task_attr);
    osKernelStart();

    for (;;)
    {
    }
}

static void display_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        lcd12864_show_string(0U, 0U, "STM32 + FreeRTOS");
        lcd12864_show_string(1U, 0U, "LCD12864 in Keil5");
        osDelay(500U);
    }
}

static void lcd12864_init(void)
{
    /* Hardware-specific initialization should be implemented here. */
}

static void lcd12864_clear(void)
{
    /* Hardware-specific clear should be implemented here. */
}

static void lcd12864_show_string(uint8_t row, uint8_t col, const char *text)
{
    (void)row;
    (void)col;
    (void)text;
    /* Hardware-specific draw routine should be implemented here. */
}
