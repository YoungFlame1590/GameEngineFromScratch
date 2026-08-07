#include <Windows.h>  // Win32 core API: windows, messages, GDI, etc.
#include <windowsx.h> // Helper macros for common window message handling (e.g. GET_X_LPARAM)
#include <tchar.h>    // TCHAR macro: generic character type mapping for ANSI/Unicode

// Forward declaration of the window procedure (callback); registered to the window class in WinMain
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// WinMain: entry point of a Windows GUI program
// hInstance     Handle to the current program instance
// hPrevInstance Deprecated parameter, always NULL (legacy from 16-bit Windows)
// lpCmdLine     Command-line argument string
// nCmdShow      Initial window display mode (maximized/minimized/normal, etc.)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPTSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;     // Handle to the created window
    WNDCLASSEX wc; // Window class structure describing the window's behavior and appearance

    // Zero out the window class structure to avoid leftover garbage data
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);           // Size of the structure, used by the system for version checking
    wc.style = CS_HREDRAW | CS_VREDRAW;       // Repaint automatically when the window is resized (horizontally/vertically)
    wc.lpfnWndProc = WindowProc;              // Message handler for this window class
    wc.hInstance = hInstance;                 // Associate with the current program instance
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Use the system default arrow cursor
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;  // Window background uses the system window color (white)
    wc.lpszClassName = _T("WindowClass1");    // Window class name, referenced by name when creating windows

    // Register the window class with the system before CreateWindowEx can create instances
    RegisterClassEx(&wc);

    // Create a window instance:
    // The WS_OVERLAPPEDWINDOW style combines the title bar, borders, system menu, and minimize/maximize buttons
    // Initial position (300, 300), size 500 x 400 pixels
    hWnd = CreateWindowEx(0, _T("WindowClass1"), _T("Hello, Engine!"),
                          WS_OVERLAPPEDWINDOW,
                          300, 300, 500, 400, NULL, NULL, hInstance, NULL);

    // Display the window in the mode specified by nCmdShow
    ShowWindow(hWnd, nCmdShow);

    MSG msg; // Message structure holding system messages retrieved from the queue

    // Message loop: GetMessage retrieves a message from the queue; returns 0 on WM_QUIT to exit the loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg); // Convert key messages into character messages (WM_CHAR)

        DispatchMessage(&msg); // Dispatch the message to the window procedure of the corresponding window
    }

    // When the loop exits, return the exit code carried by the WM_QUIT message (usually 0)
    return msg.wParam;
}

// Window procedure: all messages sent to this window are handled here
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rec = {20, 20, 60, 80};
        HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);

        FillRect(hdc, &rec, brush);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY: // Received when the user clicks the close button and the window is about to be destroyed
    {
        // Post WM_QUIT to the message queue so GetMessage above returns 0 and the message loop ends
        PostQuitMessage(0);
        return 0;
    }
    break;
    }

    // Hand unhandled messages to the system default handler (handles minimize, painting, resizing, etc.)
    return DefWindowProc(hWnd, message, wParam, lParam);
}
