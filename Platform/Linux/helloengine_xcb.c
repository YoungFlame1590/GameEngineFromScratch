/*
 * helloengine_xcb.c
 *
 * 使用 XCB（X C Binding，X 协议的 C 语言绑定）创建窗口的入门示例。
 * 功能流程：
 *   1. 连接 X 服务器，获取默认屏幕；
 *   2. 创建图形上下文（GC）和主窗口，设置窗口标题；
 *   3. 映射窗口使其可见，进入事件循环；
 *   4. 按下任意键后退出程序。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>

int main(void)
{
    /* ---- 变量声明 ---- */
    xcb_connection_t *pConn;     /* 到 X 服务器的连接 */
    xcb_screen_t *pScreen;       /* 默认屏幕信息（尺寸、深度、根窗口等） */
    xcb_window_t window;         /* 窗口 ID */
    xcb_gcontext_t foreground;   /* 前景图形上下文（GC，决定绘制使用的颜色等） */
    xcb_gcontext_t background;   /* 背景图形上下文 */
    xcb_generic_event_t *pEvent; /* 从 X 服务器接收的事件 */
    uint32_t mask = 0;           /* 属性掩码，指明 values 中哪些字段有效 */
    uint32_t values[2];          /* 与掩码对应的属性值数组 */
    uint8_t isQuit = 0;          /* 退出标志，置 1 时结束事件循环 */

    char title[] = "Hello Engine!";                  /* 窗口标题 */
    char title_icon[] = "Hello Engine! (iconified)"; /* 最小化时显示的标题 */

    /* ---- 建立与 X 服务器的连接，并取得默认屏幕 ---- */
    /* 第一个参数为显示名，NULL(0) 表示使用 $DISPLAY 环境变量指定的显示器；
       第二个参数是错误指针，NULL(0) 表示忽略连接错误。 */
    pConn = xcb_connect(0, 0);
    /* 从连接信息中取出根屏幕，作为后续创建窗口、GC 的默认目标 */
    pScreen = xcb_setup_roots_iterator(xcb_get_setup(pConn)).data;

    /* ---- 创建前景图形上下文（GC） ---- */
    /* 创建 GC 需要一个目标窗口，这里先使用根窗口 */
    window = pScreen->root;

    foreground = xcb_generate_id(pConn);                  /* 向服务器申请一个新的资源 ID */
    mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES; /* 设置字段：前景色 + 是否记录图形曝光 */
    values[0] = pScreen->black_pixel;                     /* 前景色设为黑色 */
    values[1] = 0;                                        /* GRAPHICS_EXPOSURES 为 0，不记录图形曝光事件 */
    xcb_create_gc(pConn, foreground, window, mask, values);

    /* ---- 创建背景图形上下文（GC），背景色设为白色 ---- */
    background = xcb_generate_id(pConn);
    mask = XCB_GC_BACKGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = pScreen->white_pixel; /* 背景色设为白色 */
    values[1] = 0;
    xcb_create_gc(pConn, background, window, mask, values);

    /* ---- 创建真正的应用窗口 ---- */
    window = xcb_generate_id(pConn);                                /* 为窗口重新申请资源 ID */
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;                   /* 设置字段：背景像素 + 事件掩码 */
    values[0] = pScreen->white_pixel;                               /* 窗口背景为白色 */
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS; /* 关注"窗口曝光"与"按键"事件 */
    xcb_create_window(pConn, XCB_COPY_FROM_PARENT,                  /* 继承父窗口的深度、类别和视觉 */
                      window, pScreen->root,                        /* 新窗口 ID，父窗口为根窗口 */
                      20, 20, 640, 480, 10,                         /* 位于(20,20)，尺寸 640x480，边框宽 10 像素 */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,                /* 窗口类别：既可显示内容也可接收输入 */
                      pScreen->root_visual, mask, values);

    /* ---- 设置窗口标题属性 ---- */
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, /* 以 REPLACE 模式覆盖属性旧值 */
                        window,
                        XCB_ATOM_WM_NAME,   /* 窗口管理器使用的标题属性 */
                        XCB_ATOM_STRING, 8, /* 属性类型为字符串，每个元素占 8 位 */
                        strlen(title), title);

    /* 设置窗口最小化时（图标化）显示的标题 */
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, window,
                        XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
                        strlen(title_icon), title_icon);

    /* ---- 将窗口映射到屏幕（使其可见）并刷新请求 ---- */
    xcb_map_window(pConn, window);
    /* 把此前累积的所有请求一次性发送给 X 服务器（XCB 默认异步排队） */
    xcb_flush(pConn);

    /* ---- 事件循环 ---- */
    /* 阻塞等待服务器事件；连接出错（返回 NULL）或 isQuit 置位时退出循环 */
    while ((pEvent = xcb_wait_for_event(pConn)) && !isQuit)
    {
        /* 事件类型保存在 response_type 的低 7 位，最高位表示事件是否由其他客户端发送；
           用 &~0x80（等价于 &0x7f）掩掉发送标志位，即可得到真实事件类型 */
        switch (pEvent->response_type & ~0x80)
        {
        case XCB_EXPOSE: /* 窗口内容暴露，需要重绘（本示例未绘制内容，故直接忽略） */
        {
            xcb_rectangle_t rect = {20, 20, 60, 80};
            xcb_poly_fill_rectangle(pConn, window, foreground, 1, &rect);
            xcb_flush(pConn);
        }
        break;
        case XCB_KEY_PRESS: /* 任意按键：置退出标志，结束程序 */
            isQuit = 1;
            break;
        }
        free(pEvent); /* 每个事件由 xcb_wait_for_event 分配内存，使用后必须释放 */
    }

    xcb_disconnect(pConn);

    return 0;
}
