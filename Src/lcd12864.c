#include "lcd12864.h"
#include <string.h>

static void lcd_write_text_line(uint8_t row, const char *line)
{
    LCD12864_SetCursor(row, 0U);
    for (uint8_t i = 0U; i < LCD_COLS; ++i) {
        LCD12864_WriteChar(line[i]);
    }
}

void LCD12864_Init(void)
{
    LCD12864_DelayUs(40000U);
    LCD12864_WriteCommand(0x30U);
    LCD12864_DelayUs(100U);
    LCD12864_WriteCommand(0x0CU);
    LCD12864_DelayUs(100U);
    LCD12864_WriteCommand(0x01U);
    LCD12864_DelayUs(2000U);
}

void LCD12864_Clear(void)
{
    LCD12864_WriteCommand(0x01U);
    LCD12864_DelayUs(2000U);
}

void LCD12864_SetCursor(uint8_t row, uint8_t col)
{
    static const uint8_t base_addr[LCD_TEXT_ROWS] = {0x80U, 0x90U, 0x88U, 0x98U};
    if ((row >= LCD_TEXT_ROWS) || (col >= LCD_COLS)) {
        return;
    }
    LCD12864_WriteCommand((uint8_t)(base_addr[row] + col));
}

void LCD12864_WriteChar(char c)
{
    LCD12864_WriteData((uint8_t)c);
}

void LCD12864_WriteString(const char *s)
{
    while (*s != '\0') {
        LCD12864_WriteChar(*s++);
    }
}

void LCD12864_RenderFrame(const DisplayFrame *frame)
{
    char row_buf[LCD_COLS + 1U];
    memset(row_buf, ' ', LCD_COLS);
    row_buf[LCD_COLS] = '\0';
    strncpy(row_buf, frame->status, LCD_COLS);
    lcd_write_text_line(0U, row_buf);
    lcd_write_text_line(1U, frame->lines[0]);
    lcd_write_text_line(2U, frame->lines[1]);
    lcd_write_text_line(3U, frame->lines[2]);
    LCD12864_SetCursor((uint8_t)(1U + frame->cursor_line), frame->cursor_col);
}

__attribute__((weak)) void LCD12864_DelayUs(uint32_t us)
{
    (void)us;
}

__attribute__((weak)) void LCD12864_WriteCommand(uint8_t cmd)
{
    (void)cmd;
}

__attribute__((weak)) void LCD12864_WriteData(uint8_t data)
{
    (void)data;
}
