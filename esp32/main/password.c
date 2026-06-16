#include "password.h"
#include "lcd.h"
#include "gui.h"
#include "touch.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PWD_MAX     6
#define DEFAULT_PWD "1234"

// 按键布局
#define KEY_X0      8
#define KEY_Y0      90
#define BTN_W       68
#define BTN_H       46
#define BTN_GAP     10
#define COLS        3
#define ROWS        4

static const char *btn_labels[ROWS][COLS] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"DEL", "0", "OK"}
};

static char input_buf[PWD_MAX + 1];
static u8 input_len = 0;

//==============================================================================
// 绘制单个按键
//==============================================================================
static void draw_btn(u8 row, u8 col, u16 color)
{
    u16 x = KEY_X0 + col * (BTN_W + BTN_GAP);
    u16 y = KEY_Y0 + row * (BTN_H + BTN_GAP);
    u16 x2 = x + BTN_W;
    u16 y2 = y + BTN_H;

    LCD_Fill(x, y, x2, y2, color);
    POINT_COLOR = WHITE;
    LCD_DrawRectangle(x, y, x2, y2);

    const char *label = btn_labels[row][col];
    u8 len = strlen(label);
    u16 tx = x + (BTN_W - len * 16) / 2;
    u16 ty = y + (BTN_H - 16) / 2;
    Show_Str(tx, ty, WHITE, color, (u8 *)label, 16, 1);
}

//==============================================================================
// 绘制数字键盘
//==============================================================================
static void draw_keypad(void)
{
    for (u8 r = 0; r < ROWS; r++)
        for (u8 c = 0; c < COLS; c++)
            draw_btn(r, c, BLUE);
}

//==============================================================================
// 绘制密码输入点
//==============================================================================
static void draw_dots(void)
{
    // 清空输入显示区
    LCD_Fill(10, 35, lcddev.width - 10, 75, WHITE);
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(10, 35, lcddev.width - 10, 75);

    // 显示已输入的密码点数
    if (input_len > 0) {
        u16 start = (lcddev.width - input_len * 24) / 2;
        for (u8 i = 0; i < input_len; i++)
            gui_circle(start + i * 24, 55, BLACK, 6, 1);
    }

    POINT_COLOR = BLACK;
    LCD_ShowString(10, 46, 16, (u8 *)"Password:", 1);
}

//==============================================================================
// 碰撞检测 - 返回按键索引 (1~12) 或 0
//==============================================================================
static u8 hit_test(u16 tx, u16 ty)
{
    for (u8 r = 0; r < ROWS; r++) {
        for (u8 c = 0; c < COLS; c++) {
            u16 x1 = KEY_X0 + c * (BTN_W + BTN_GAP);
            u16 y1 = KEY_Y0 + r * (BTN_H + BTN_GAP);
            if (tx >= x1 && tx <= x1 + BTN_W &&
                ty >= y1 && ty <= y1 + BTN_H) {
                return r * COLS + c + 1;
            }
        }
    }
    return 0;
}

//==============================================================================
// 底部状态栏（正确/错误提示）
//==============================================================================
static void show_result(u8 success)
{
    u16 color = success ? GREEN : RED;
    u16 y1 = lcddev.height - 30;
    u16 y2 = lcddev.height - 5;

    LCD_Fill(10, y1, lcddev.width - 10, y2, color);
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(10, y1, lcddev.width - 10, y2);
    POINT_COLOR = WHITE;
    LCD_ShowString(70, y1 + 4, 16,
                   (u8 *)(success ? "UNLOCKED!" : "WRONG"), 1);
}

//==============================================================================
// 重置UI（清空输入，完全重绘）
//==============================================================================
static void reset_ui(void)
{
    input_len = 0;
    memset(input_buf, 0, sizeof(input_buf));

    LCD_Clear(WHITE);

    // 标题
    POINT_COLOR = BLUE;
    LCD_ShowString(45, 8, 16, (u8 *)"PASSWORD LOCK", 1);

    // 密码显示区
    draw_dots();

    // 数字键盘
    draw_keypad();

    // 底部状态栏
    LCD_Fill(10, lcddev.height - 30, lcddev.width - 10,
             lcddev.height - 5, WHITE);
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(10, lcddev.height - 30, lcddev.width - 10,
                      lcddev.height - 5);
    POINT_COLOR = GRAY;
    LCD_ShowString(50, lcddev.height - 26, 16, (u8 *)"Enter PIN", 1);
}

//==============================================================================
// 密码锁主函数
//==============================================================================
void Password_Lock(void)
{
    TP_Init();

    // 强制使用默认映射（忽略旧校准数据，避免因屏幕方向改变导致触控偏移）
    tp_dev.xfac = 0;
    tp_dev.yfac = 0;

    reset_ui();

    u8 was_pressed = 0;

    while (1)
    {
        u8 pressed = tp_dev.scan(0) & TP_PRES_DOWN;

        if (pressed && !was_pressed)
        {
            // 检测到新按下事件
            u16 tx = tp_dev.x;
            u16 ty = tp_dev.y;
            u8 hit = hit_test(tx, ty);

            if (hit > 0) {
                u8 idx = hit - 1;
                u8 row = idx / COLS;
                u8 col = idx % COLS;
                const char *label = btn_labels[row][col];

                // 按键高亮反馈
                draw_btn(row, col, GRAYBLUE);
                vTaskDelay(pdMS_TO_TICKS(100));
                draw_btn(row, col, BLUE);

                // ---- 处理按键 ----
                if (strcmp(label, "DEL") == 0) {
                    if (input_len > 0) {
                        input_len--;
                        input_buf[input_len] = '\0';
                        draw_dots();
                    }
                }
                else if (strcmp(label, "OK") == 0) {
                    if (input_len > 0) {
                        if (strcmp(input_buf, DEFAULT_PWD) == 0) {
                            show_result(1);
                            vTaskDelay(pdMS_TO_TICKS(300));

                            // 解锁成功 → 全屏欢迎画面
                            LCD_Clear(GREEN);
                            POINT_COLOR = WHITE;
                            Show_Str(30, 100, WHITE, GREEN,
                                     (u8 *)"UNLOCKED!", 24, 1);
                            LCD_ShowString(30, 145, 16,
                                           (u8 *)"Press RST to lock", 1);
                            while (1)
                                vTaskDelay(pdMS_TO_TICKS(100));
                        } else {
                            show_result(0);
                            vTaskDelay(pdMS_TO_TICKS(1500));
                            reset_ui();
                        }
                    }
                }
                else if (input_len < PWD_MAX) {
                    input_buf[input_len++] = label[0];
                    draw_dots();
                }
            }
        }

        was_pressed = pressed;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
