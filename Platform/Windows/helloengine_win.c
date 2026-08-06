#include <Windows.h>      // Win32 核心 API：窗口、消息、GDI 等
#include <windowsx.h>     // 常用的窗口消息处理辅助宏（如 GET_X_LPARAM 等）
#include <tchar.h>        // TCHAR 宏：支持 ANSI/Unicode 的通用字符类型映射

// 窗口过程（回调函数）前向声明，WinMain 中将其注册给窗口类
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// WinMain：Windows 图形界面程序的入口点
// hInstance    当前程序实例的句柄
// hPrevInstance 已废弃参数，始终为 NULL（16 位 Windows 遗留）
// lpCmdLine    命令行参数字符串
// nCmdShow     窗口初始显示方式（最大化/最小化/正常等）
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPTSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;        // 创建出的窗口句柄
    WNDCLASSEX wc;    // 窗口类结构体，描述窗口的行为与外观

    // 将窗口类结构体全部清零，避免残留垃圾数据
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);          // 结构体大小，供系统校验版本
    wc.style = CS_HREDRAW | CS_VREDRAW;      // 窗口尺寸变化时（水平/垂直）自动重绘
    wc.lpfnWndProc = WindowProc;             // 指定该窗口类的消息处理函数
    wc.hInstance = hInstance;                // 关联当前程序实例
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // 使用系统默认箭头光标
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;  // 窗口背景用系统窗口色（白色）
    wc.lpszClassName = _T("WindowClass1");   // 窗口类名称，创建窗口时按名引用

    // 向系统注册窗口类，之后才能用 CreateWindowEx 创建实例
    RegisterClassEx(&wc);

    // 创建窗口实例：
    // 样式 WS_OVERLAPPEDWINDOW 组合了标题栏、边框、系统菜单、最小化/最大化按钮
    // 初始位置 (300, 300)，尺寸 500 x 400 像素
    hWnd = CreateWindowEx(0, _T("WindowClass1"), _T("Hello, Engine!"),
                          WS_OVERLAPPEDWINDOW,
                          300, 300, 500, 400, NULL, NULL, hInstance, NULL);

    // 按 nCmdShow 指定的方式把窗口显示出来
    ShowWindow(hWnd, nCmdShow);

    MSG msg; // 消息结构体，存放从队列中取出的系统消息

    // 消息循环：GetMessage 从队列取消息，收到 WM_QUIT 时返回 0 退出循环
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);   // 将按键消息转换为字符消息（WM_CHAR）

        DispatchMessage(&msg);    // 把消息分发给对应窗口的窗口过程处理
    }

    // 退出循环时返回 WM_QUIT 消息携带的退出码（通常为 0）
    return msg.wParam;
}

// 窗口过程：所有发送给该窗口的消息都会在这里被处理
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY: // 用户点击关闭按钮，窗口即将销毁时收到此消息
    {
        // 向消息队列投递 WM_QUIT，使上方 GetMessage 返回 0，结束消息循环
        PostQuitMessage(0);
        return 0;
    }
    break;
    }

    // 未处理的消息交给系统默认处理函数（处理最小化、绘制、调整大小等）
    return DefWindowProc(hWnd, message, wParam, lParam);
}
