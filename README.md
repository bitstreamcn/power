# power
可调电源，使用Azure RTOS + GUIX显示界面，MCU为R7FA4M2AD3CFL

![Image text](https://github.com/bitstreamcn/power/blob/master/images/IMG_20251007_085554.jpg)


开发板使用的MCU型号是R7FA4M2AD3CFL，瑞萨开发环境中FSP默认不支持guix组件（勾选guix但是编译不通过），参考本项目可以让GUIX在R7FA4M2AD3CFL上使用。

 

配置GUIX步骤：

 

1、在e2 studio创建工程，创建工程的时候选择Azure RTOS

2、打开FSP配置界面，切换到Components页面，勾选Microsoft->Azure->GUIX->gx；Renesas->TES->all->dave2d

3、下载GitHub - bitstreamcn/power: 可调电源源码，把ra->fsp->inc->instances->gx_port.h文件拷贝到相应的目录ra/fsp/inc/instances/

4、在微软应用商店下载guix studio

5、使用guix studio创建项目，导入中文字体，导入的时候要勾选Indlude character set defined by String Table，这样就可以在ui中显示中文了

6、使用guix studio设计完界面后，使用Generate All Output Files菜单生成c文件，把生成的文件拷贝到工程源码目录下

7、创建显示驱动函数

```
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
```

8、在程序的入口处创建窗口，代码如下：

```
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

    /* let GUIX run! */
    ret = gx_system_start ();
    if (ret != GX_SUCCESS)
    {
        printf ("gx_system_start fail.\n");
        return;
    }
```

 

 

guix开发资料可以参考：

https://forum.anfulai.cn/forum.php?mod=viewthread&tid=98429

guix文档 https://github.com/eclipse-threadx/rtos-docs




