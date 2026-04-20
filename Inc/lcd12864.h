#ifndef LCD12864_H
#define LCD12864_H

#include <stdint.h>

#define LCD_COLS 16U
#define LCD_TEXT_ROWS 4U
#define LCD_INPUT_ROWS 3U

typedef struct {
    char status[LCD_COLS + 1U];
    char lines[LCD_INPUT_ROWS][LCD_COLS + 1U];
    uint8_t cursor_line;
    uint8_t cursor_col;
} DisplayFrame;

void LCD12864_Init(void);
void LCD12864_Clear(void);
void LCD12864_SetCursor(uint8_t row, uint8_t col);
void LCD12864_WriteChar(char c);
void LCD12864_WriteString(const char *s);
void LCD12864_RenderFrame(const DisplayFrame *frame);

/* 底层总线接口（可在 board 文件中重写） */
void LCD12864_DelayUs(uint32_t us);
void LCD12864_WriteCommand(uint8_t cmd);
void LCD12864_WriteData(uint8_t data);

#endif
