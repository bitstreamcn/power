#include <ui_thread.h>
#include "r_transfer_api.h"
extern "C" {
#include <stdio.h>
#include "tx_api.h"
#include "gx_api.h"

#include "hal_data.h"

#include "gx_system.h"
#include "gx_display.h"
#include "gx_utility.h"
#include "oled.h"
#include "qe_touch_config.h"

}
#include "./guix/power_supply_resources.h"
#include "./guix/power_supply_specifications.h"

FSP_CPP_HEADER

//参考资料
// https://gitee.com/CoreMaker/RA4M2
// https://forum.anfulai.cn/forum.php?mod=viewthread&tid=98429
//guix文档 https://github.com/eclipse-threadx/rtos-docs

/* The root window is a special GUIX background window, right on top of the canvas. */
GX_WINDOW_ROOT *root_window;

GX_WIDGET *main_window_widget;
GX_WIDGET *volt_window_widget;
GX_WIDGET *current_window_widget;

GX_WIDGET *display_widget;

extern float in_volt;
extern float out_volt;
extern float out_current;

extern ULONG display_1_canvas_memory[256];

float out_volt_set_value = 3.3f;
float out_current_set_value = 0.1f;

bool adj_volt = false;
bool adj_current = false;

void OnLeftTouch();
void OnRightTouch();
void OnK2Click();
void OnK3Click();
void OnK4Click();

static void ra4_graphics_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty);
UINT ra4_graphics_driver_setup(GX_DISPLAY *display);
static void ra4_graphics_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty)
{
    uint8_t *p;
    /* 防止警告 */
    (void) canvas;
    (void) dirty;

    /*获得OLED画布的地址 */
    p = (uint8_t*) display_1_canvas_memory;
    (void) p;
    /*将画布的内容绘制到OLED显存*/
    for (u8 y = 0; y < DISPLAY_1_Y_RESOLUTION; y++)
    {
        for (u8 x = 0; x < DISPLAY_1_X_RESOLUTION; x += 8)
        {
            OLED_PutPixel (x, y, (p[16 * y + x / 8] & (0x80)) >> 7);
            OLED_PutPixel (x + 1, y, (p[16 * y + x / 8] & (0x40)) >> 6);
            OLED_PutPixel (x + 2, y, (p[16 * y + x / 8] & (0x20)) >> 5);
            OLED_PutPixel (x + 3, y, (p[16 * y + x / 8] & (0x10)) >> 4);
            OLED_PutPixel (x + 4, y, (p[16 * y + x / 8] & (0x08)) >> 3);
            OLED_PutPixel (x + 5, y, (p[16 * y + x / 8] & (0x04)) >> 2);
            OLED_PutPixel (x + 6, y, (p[16 * y + x / 8] & (0x02)) >> 1);
            OLED_PutPixel (x + 7, y, (p[16 * y + x / 8] & (0x01)) >> 0);
        }
    }
    /*将OLED显存的内容实际绘制到OLED*/
    OLED_Refresh ();
}

UINT ra4_graphics_driver_setup(GX_DISPLAY *display)
{
    _gx_display_driver_monochrome_setup (display, GX_NULL, ra4_graphics_buffer_toggle);
    return (GX_SUCCESS);
}

/* New Thread entry function */
void ui_thread_entry(void)
{

    fsp_err_t err;

    uint64_t button_status;
    g_qe_touch_flag = 0;
    err = RM_TOUCH_Open (g_qe_touch_instance_config01.p_ctrl, g_qe_touch_instance_config01.p_cfg);
    assert(FSP_SUCCESS == err);
    err = RM_TOUCH_ScanStart (g_qe_touch_instance_config01.p_ctrl);
    assert(FSP_SUCCESS == err);

    /* 初始化 SPI 驱动 */
    err = R_SCI_SPI_Open (&g_spi0_ctrl, &g_spi0_cfg);
    assert(FSP_SUCCESS == err);

    OLED_Init ();

    for (unsigned char i = 15; i < 30; i += 4)
    {
        OLED_DrawCircle (63, 31, i);
        OLED_Refresh ();
        R_BSP_SoftwareDelay (50, BSP_DELAY_UNITS_MILLISECONDS);
    }
    R_BSP_SoftwareDelay (500, BSP_DELAY_UNITS_MILLISECONDS);
    //OLED_Clear();

    /* Initialize GUIX. */
    gx_system_initialize ();

    UINT ret = gx_studio_display_configure (DISPLAY_1, ra4_graphics_driver_setup, LANGUAGE_CHINESE, DISPLAY_1_THEME_1,
                                            &root_window);
    if (ret != GX_SUCCESS)
    {
        printf ("gx_studio_display_configure fail.\n");
        return;
    }

    /* Create the screen - attached to root window. */
    ret = gx_studio_named_widget_create ((char*) "volt_window", (GX_WIDGET*) root_window, &volt_window_widget);
    if (ret != GX_SUCCESS)
    {
        printf ("gx_studio_named_widget_create fail.\n");
        return;
    }

    ret = gx_studio_named_widget_create ((char*) "current_window", (GX_WIDGET*) root_window, &current_window_widget);
    if (ret != GX_SUCCESS)
    {
        printf ("gx_studio_named_widget_create fail.\n");
        return;
    }

    ret = gx_studio_named_widget_create ((char*) "main_window", (GX_WIDGET*) root_window, &main_window_widget);
    if (ret != GX_SUCCESS)
    {
        printf ("gx_studio_named_widget_create fail.\n");
        return;
    }

    /* Show the root window. */
    ret = gx_widget_show(root_window);
    if (ret != GX_SUCCESS)
    {
        printf ("gx_widget_show fail.\n");
        return;
    }

    gx_widget_hide(volt_window_widget);
    gx_widget_hide(current_window_widget);

    /* let GUIX run! */
    ret = gx_system_start ();
    if (ret != GX_SUCCESS)
    {
        printf ("gx_system_start fail.\n");
        return;
    }

    GX_PROMPT *label_current = GX_NULL;
    GX_PROMPT *label_in_v = GX_NULL;
    GX_PROMPT *label_out_v = GX_NULL;
    GX_PROMPT *label_a_ma = GX_NULL;
    gx_widget_find(main_window_widget, current, 1, (GX_WIDGET** )&label_current);
    gx_widget_find(main_window_widget, in_v, 1, (GX_WIDGET** )&label_in_v);
    gx_widget_find(main_window_widget, out_v, 1, (GX_WIDGET** )&label_out_v);
    gx_widget_find(main_window_widget, a_ma, 1, (GX_WIDGET** )&label_a_ma);

    GX_PROMPT *label_current_set = GX_NULL;
    GX_PROMPT *label_out_v_set = GX_NULL;
    gx_widget_find(volt_window_widget, out_volt_set, 1, (GX_WIDGET** )&label_out_v_set);
    gx_widget_find(current_window_widget, max_current, 1, (GX_WIDGET** )&label_current_set);

    display_widget = main_window_widget;

    float local_in_volt = 0;
    float local_out_volt = 0;
    float local_out_current = 0;

    GX_CHAR prompt_current[20] =
    { 0 };
    GX_CHAR prompt_in[20] =
    { 0 };
    GX_CHAR prompt_out[20] =
    { 0 };

    GX_CHAR prompt_volt_set[20] =
    { 0 };
    GX_CHAR prompt_current_set[20] =
    { 0 };
    /* TODO: add your own code here */
    bool need_refresh = false;
    while (1)
    {
        //检测到数据变动就更新UI
        if (abs (local_out_current - out_current) > 0.0001 && display_widget == main_window_widget)
        {
            local_out_current = out_current;
            snprintf (prompt_current, 20, "%0.3f", local_out_current);
            gx_prompt_text_set (label_current, prompt_current);
            gx_system_dirty_mark((GX_WIDGET* )label_current);
            need_refresh = true;
        }

        if (abs (local_in_volt - in_volt) > 0.001 && display_widget == main_window_widget)
        {
            local_in_volt = in_volt;
            snprintf (prompt_in, 20, "%0.3f", local_in_volt);
            gx_prompt_text_set (label_in_v, prompt_in);
            gx_system_dirty_mark((GX_WIDGET* )label_in_v);
            need_refresh = true;
        }

        if (abs (local_out_volt - out_volt) > 0.001 && display_widget == main_window_widget)
        {
            local_out_volt = out_volt;
            snprintf (prompt_out, 20, "%0.3f", local_out_volt);
            gx_prompt_text_set (label_out_v, prompt_out);
            gx_system_dirty_mark((GX_WIDGET* )label_out_v);
            need_refresh = true;
        }

        if (display_widget == volt_window_widget)
        {
            static float local_out_volt_set_value = 0;
            if (abs (local_out_volt_set_value - out_volt_set_value) > 0.01)
            {
                local_out_volt_set_value = out_volt_set_value;
                snprintf (prompt_volt_set, 20, "%0.1f", local_out_volt_set_value);
                gx_prompt_text_set (label_out_v_set, prompt_volt_set);
                gx_system_dirty_mark((GX_WIDGET* )label_out_v_set);
                need_refresh = true;
            }
        }
        if (display_widget == current_window_widget)
        {
            static float local_out_current_set_value = 0;
            if (abs (local_out_current_set_value - out_current_set_value) > 0.01)
            {
                local_out_current_set_value = out_current_set_value;
                snprintf (prompt_current_set, 20, "%0.3f", local_out_current_set_value);
                gx_prompt_text_set (label_current_set, prompt_current_set);
                gx_system_dirty_mark((GX_WIDGET* )label_current_set);
                need_refresh = true;
            }
        }

        if (need_refresh)
        {
            gx_system_canvas_refresh ();
            need_refresh = false;
        }

        if (g_qe_touch_flag)
        {
            err = RM_TOUCH_DataGet (g_qe_touch_instance_config01.p_ctrl, &button_status, NULL, NULL);
            if (FSP_SUCCESS == err)
            {
                static bool right_down = false;
                static bool left_down = false;
                if (button_status & (0b1))
                {
                    if (!right_down)
                    {
                        //处理按下事件
                        OnRightTouch ();
                    }
                    //右边触摸
                    right_down = true;
                }
                if ((button_status & (0b1)) == 0)
                {
                    //右边触摸抬起
                    if (right_down)
                    {
                        g_qe_touch_flag = 0; //用于调试下断点
                    }
                    right_down = false;
                }
                if (button_status & (0b10))
                {
                    if (!left_down)
                    {
                        //处理按下事件
                        OnLeftTouch ();
                    }
                    //左边触摸
                    left_down = true;
                }
                if ((button_status & (0b10)) == 0)
                {
                    //左边触摸抬起
                    if (left_down)
                    {
                        g_qe_touch_flag = 0; //用于调试下断点
                    }
                    left_down = false;
                }
            }
            g_qe_touch_flag = 0;
            err = RM_TOUCH_ScanStart (g_qe_touch_instance_config01.p_ctrl);
        }

        //处理按键
        //K4
        bsp_io_level_t p_port_value_pin_013;
        static int pin_013_down_times = 0;
        static bool pin_013_first_down = true;
        err = R_IOPORT_PinRead (&g_ioport_ctrl, BSP_IO_PORT_00_PIN_13, &p_port_value_pin_013);
        if (FSP_SUCCESS == err)
        {
            if (p_port_value_pin_013)        //按下按键P112，P103亮
            {
                pin_013_down_times++;
                if (pin_013_down_times == 5)
                {
                    if (!pin_013_first_down)
                    {
                        //处理按键事件
                        OnK4Click ();
                    }
                    pin_013_first_down = false;
                }
            }
            else
            {
                pin_013_down_times = 0;
            }
        }

        //K3
        bsp_io_level_t p_port_value_pin_112;
        static int pin_112_down_times = 0;
        static bool pin_112_first_down = true;
        err = R_IOPORT_PinRead (&g_ioport_ctrl, BSP_IO_PORT_01_PIN_12, &p_port_value_pin_112);
        if (FSP_SUCCESS == err)
        {
            if (p_port_value_pin_112)        //按下按键P112，P103亮
            {
                pin_112_down_times++;
                if (pin_112_down_times == 5)
                {
                    if (!pin_112_first_down)
                    {
                        //处理按键事件
                        OnK3Click ();
                    }
                    pin_112_first_down = false;
                }
            }
            else
            {
                pin_112_down_times = 0;
            }
        }
        //K2
        bsp_io_level_t p_port_value_pin_111;
        static int pin_111_down_times = 0;
        static bool pin_111_first_down = true;
        err = R_IOPORT_PinRead (&g_ioport_ctrl, BSP_IO_PORT_01_PIN_11, &p_port_value_pin_111);
        if (FSP_SUCCESS == err)
        {
            if (p_port_value_pin_111)        //按下按键P112，P103亮
            {
                pin_111_down_times++;
                if (pin_111_down_times == 5)
                {
                    if (!pin_111_first_down)
                    {
                        //处理按键事件
                        OnK2Click ();
                    }
                    pin_111_first_down = false;
                }
            }
            else
            {
                pin_111_down_times = 0;
            }
        }

        tx_thread_sleep (1);
    }
}

void OnLeftTouch()
{
    if (display_widget == volt_window_widget)
    {
        out_volt_set_value -= 0.1f;
        adj_volt = true;
    }
    if (display_widget == current_window_widget)
    {
        out_current_set_value -= 0.1f;
        adj_current = true;
    }
}
void OnRightTouch()
{
    if (display_widget == volt_window_widget)
    {
        out_volt_set_value += 0.1f;
        adj_volt = true;
    }
    if (display_widget == current_window_widget)
    {
        out_current_set_value += 0.1f;
        adj_current = true;
    }
}

void OnK2Click()
{
    if (display_widget == main_window_widget)
    {
        display_widget = volt_window_widget;
        gx_widget_show(volt_window_widget);
        gx_widget_hide(main_window_widget);
        gx_system_canvas_refresh ();
    }
    else if (display_widget == volt_window_widget)
    {
        display_widget = main_window_widget;
        gx_widget_show(main_window_widget);
        gx_widget_hide(volt_window_widget);
        gx_system_canvas_refresh ();
    }
    else if (display_widget == current_window_widget)
    {
        display_widget = volt_window_widget;
        gx_widget_show(volt_window_widget);
        gx_widget_hide(current_window_widget);
        gx_system_canvas_refresh ();
    }
}
void OnK3Click()
{
    if (abs (out_volt_set_value - 3.3) < 0.01)
    {
        out_volt_set_value = 5.0f;
    }
    else if (abs (out_volt_set_value - 5.0) < 0.01)
    {
        out_volt_set_value = 1.8f;
    }
    else
    {
        out_volt_set_value = 3.3f;
    }
    adj_volt = true;
}
void OnK4Click()
{
    if (display_widget == main_window_widget)
    {
        display_widget = current_window_widget;
        gx_widget_show(current_window_widget);
        gx_widget_hide(main_window_widget);
        gx_system_canvas_refresh ();
    }
    else if (display_widget == volt_window_widget)
    {
        display_widget = current_window_widget;
        gx_widget_show(current_window_widget);
        gx_widget_hide(volt_window_widget);
        gx_system_canvas_refresh ();
    }
    else if (display_widget == current_window_widget)
    {
        display_widget = main_window_widget;
        gx_widget_show(main_window_widget);
        gx_widget_hide(current_window_widget);
        gx_system_canvas_refresh ();
    }
}

FSP_CPP_FOOTER

