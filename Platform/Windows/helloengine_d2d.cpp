#include <Windows.h>  // Win32 core API: windows, messages, GDI, etc.
#include <windowsx.h> // Helper macros for common window message handling (e.g. GET_X_LPARAM)
#include <tchar.h>    // TCHAR macro: generic character type mapping for ANSI/Unicode

#include <d2d1.h>

ID2D1Factory *pFactory = nullptr;
ID2D1HwndRenderTarget *pRenderTarget = nullptr;
ID2D1SolidColorBrush *pLightSlateGrayBrush = nullptr;
ID2D1SolidColorBrush *pCornflowerBlueBrush = nullptr;

template <class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = nullptr;
    }
}

HRESULT CreateGraphicsResources(HWND hWnd)
{
    HRESULT hr = S_OK;
    if (pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,
                                       rc.bottom - rc.top);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hWnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                &pLightSlateGrayBrush);
        }

        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                &pCornflowerBlueBrush);
        }
    }

    return hr;
}

void DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pLightSlateGrayBrush);
    SafeRelease(&pCornflowerBlueBrush);
}

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

    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        return -1;

    // Zero out the window class structure to avoid leftover garbage data
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);              // Size of the structure, used by the system for version checking
    wc.style = CS_HREDRAW | CS_VREDRAW;          // Repaint automatically when the window is resized (horizontally/vertically)
    wc.lpfnWndProc = WindowProc;                 // Message handler for this window class
    wc.hInstance = hInstance;                    // Associate with the current program instance
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); // Use the system default arrow cursor
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;     // Window background uses the system window color (white)
    wc.lpszClassName = _T("WindowClass1");       // Window class name, referenced by name when creating windows

    // Register the window class with the system before CreateWindowEx can create instances
    RegisterClassEx(&wc);

    // Create a window instance:
    // The WS_OVERLAPPEDWINDOW style combines the title bar, borders, system menu, and minimize/maximize buttons
    // Initial position (300, 300), size 500 x 400 pixels
    hWnd = CreateWindowEx(0, _T("WindowClass1"), _T("Hello, Engine![Direct 2D]"),
                          WS_OVERLAPPEDWINDOW,
                          100, 100, 960, 540, NULL, NULL, hInstance, NULL);

    // Display the window in the mode specified by nCmdShow
    ShowWindow(hWnd, nCmdShow);

    MSG msg; // Message structure holding system messages retrieved from the queue

    // Message loop: GetMessage retrieves a message from the queue; returns 0 on WM_QUIT to exit the loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg); // Convert key messages into character messages (WM_CHAR)

        DispatchMessage(&msg); // Dispatch the message to the window procedure of the corresponding window
    }

    CoUninitialize();

    // When the loop exits, return the exit code carried by the WM_QUIT message (usually 0)
    return msg.wParam;
}

// Window procedure: all messages sent to this window are handled here
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    bool wasHandled = false;

    switch (message)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &pFactory)))
        {
            result = -1;
            return result;
        }
        wasHandled = true;
        result = 0;
        break;

    case WM_PAINT:
    {
        HRESULT hr = CreateGraphicsResources(hWnd);
        if (SUCCEEDED(hr))
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            pRenderTarget->BeginDraw();

            pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

            D2D1_SIZE_F rtSize = pRenderTarget->GetSize();

            int width = static_cast<int>(rtSize.width);
            int height = static_cast<int>(rtSize.height);

            for (int x = 0; x < width; x += 10)
            {
                pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                    D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
                    pLightSlateGrayBrush,
                    0.5f);
            }

            for (int y = 0; y < height; y += 10)
            {
                pRenderTarget->DrawLine(
                    D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                    D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
                    pCornflowerBlueBrush,
                    0.5f);
            }

            D2D1_RECT_F rectangle1 = D2D1::RectF(
                rtSize.width / 2 - 50.0f,
                rtSize.height / 2 - 50.0f,
                rtSize.width / 2 + 50.0f,
                rtSize.height / 2 + 50.0f);

            D2D1_RECT_F rectangle2 = D2D1::RectF(
                rtSize.width / 2 - 100.0f,
                rtSize.height / 2 - 100.0f,
                rtSize.width / 2 + 100.0f,
                rtSize.height / 2 + 100.0f);

            pRenderTarget->FillRectangle(&rectangle1, pLightSlateGrayBrush);

            pRenderTarget->DrawRectangle(&rectangle2, pCornflowerBlueBrush);

            hr = pRenderTarget->EndDraw();
            if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
            {
                DiscardGraphicsResources();
            }

            EndPaint(hWnd, &ps);
        }
    }
        wasHandled = true;
        break;

    case WM_SIZE:
        if (pRenderTarget != nullptr)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,
                                           rc.bottom - rc.top);

            pRenderTarget->Resize(size);
        }
        wasHandled = true;
        break;

    case WM_DESTROY: // Received when the user clicks the close button and the window is about to be destroyed
        DiscardGraphicsResources();
        if (pFactory)
        {
            pFactory->Release();
            pFactory = nullptr;
        }

        // Post WM_QUIT to the message queue so GetMessage above returns 0 and the message loop ends
        PostQuitMessage(0);
        return 0;

        wasHandled = true;
        break;

    case WM_DISPLAYCHANGE:
        InvalidateRect(hWnd, nullptr, false);
        wasHandled = true;
        break;
    }

    // Hand unhandled messages to the system default handler (handles minimize, painting, resizing, etc.)
    if (!wasHandled)
    {
        result = DefWindowProc(hWnd, message, wParam, lParam);
    }
    return result;
}
