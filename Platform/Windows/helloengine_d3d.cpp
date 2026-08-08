#include <Windows.h>  // Win32 core API: windows, messages, GDI, etc.
#include <windowsx.h> // Helper macros for common window message handling (e.g. GET_X_LPARAM)
#include <tchar.h>    // TCHAR macro: generic character type mapping for ANSI/Unicode
#include <stdint.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

const uint32_t SCREEN_WIDTH = 960;
const uint32_t SCREEN_HEIGHT = 480;

IDXGISwapChain *g_pSwapchain = nullptr;
ID3D11Device *g_pDev = nullptr;
ID3D11DeviceContext *g_pDevcon = nullptr;

ID3D11RenderTargetView *g_pRTView = nullptr;

ID3D11InputLayout *g_pLayout = nullptr;
ID3D11VertexShader *g_pVS = nullptr;
ID3D11PixelShader *g_pPS = nullptr;

ID3D11Buffer *g_pVBuffer = nullptr;

struct VERTEX
{
    XMFLOAT3 Position;
    XMFLOAT4 Color;
};

template <class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = nullptr;
    }
}

void CreateRenderTarget()
{
    HRESULT hs;
    ID3D11Texture2D *pBackBuffer;

    g_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);

    g_pDev->CreateRenderTargetView(pBackBuffer, NULL, &g_pRTView);

    pBackBuffer->Release();

    g_pDevcon->OMSetRenderTargets(1, &g_pRTView, NULL);
}

void SetViewPort()
{
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;

    g_pDevcon->RSSetViewports(1, &viewport);
}

void InitPipeline()
{
    ID3DBlob *VS, *PS;

    D3DReadFileToBlob(L"copy.vso", &VS);
    D3DReadFileToBlob(L"copy.pso", &PS);

    g_pDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(),
                               NULL, &g_pVS);

    g_pDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(),
                              NULL, &g_pPS);

    g_pDevcon->VSSetShader(g_pVS, 0, 0);
    g_pDevcon->PSSetShader(g_pPS, 0, 0);

    D3D11_INPUT_ELEMENT_DESC ied[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
             D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
             D3D11_INPUT_PER_VERTEX_DATA, 0}};

    g_pDev->CreateInputLayout(ied, 2, VS->GetBufferPointer(),
                              VS->GetBufferSize(), &g_pLayout);

    g_pDevcon->IASetInputLayout(g_pLayout);

    VS->Release();
    PS->Release();
}

void InitGraphics()
{
    VERTEX OurVertices[] =
        {
            {XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
            {XMFLOAT3(0.45f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
            {XMFLOAT3(-0.45f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}};

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    g_pDev->CreateBuffer(&bd, NULL, &g_pVBuffer);

    D3D11_MAPPED_SUBRESOURCE ms;
    g_pDevcon->Map(g_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, OurVertices, sizeof(OurVertices));
    g_pDevcon->Unmap(g_pVBuffer, NULL);
}

HRESULT CreateGraphicsResources(HWND hWnd)
{
    HRESULT hr = S_OK;

    if (g_pSwapchain == nullptr)
    {
        DXGI_SWAP_CHAIN_DESC scd;

        ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

        scd.BufferCount = 1;
        scd.BufferDesc.Width = SCREEN_WIDTH;
        scd.BufferDesc.Height = SCREEN_HEIGHT;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hWnd;
        scd.SampleDesc.Count = 4;
        scd.Windowed = TRUE;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        const D3D_FEATURE_LEVEL FeatureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
                D3D_FEATURE_LEVEL_9_3,
                D3D_FEATURE_LEVEL_9_2,
                D3D_FEATURE_LEVEL_9_1,
            };

        D3D_FEATURE_LEVEL FeatureLevelSupported;

        HRESULT hr = S_OK;

        hr = D3D11CreateDeviceAndSwapChain(NULL,
                                           D3D_DRIVER_TYPE_HARDWARE,
                                           NULL,
                                           0,
                                           FeatureLevels,
                                           _countof(FeatureLevels),
                                           D3D11_SDK_VERSION,
                                           &scd,
                                           &g_pSwapchain,
                                           &g_pDev,
                                           &FeatureLevelSupported,
                                           &g_pDevcon);

        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDeviceAndSwapChain(NULL,
                                               D3D_DRIVER_TYPE_HARDWARE,
                                               NULL,
                                               0,
                                               &FeatureLevelSupported,
                                               1,
                                               D3D11_SDK_VERSION,
                                               &scd,
                                               &g_pSwapchain,
                                               &g_pDev,
                                               NULL,
                                               &g_pDevcon);
        }

        if (hr == S_OK)
        {
            CreateRenderTarget();
            SetViewPort();
            InitPipeline();
            InitGraphics();
        }
    }

    return hr;
}

void DiscardGraphicsResources()
{
    SafeRelease(&g_pLayout);
    SafeRelease(&g_pVS);
    SafeRelease(&g_pPS);
    SafeRelease(&g_pVBuffer);
    SafeRelease(&g_pSwapchain);
    SafeRelease(&g_pRTView);
    SafeRelease(&g_pDev);
    SafeRelease(&g_pDevcon);
}

void RenderFrame()
{
    const FLOAT clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    g_pDevcon->ClearRenderTargetView(g_pRTView, clearColor);

    {
        UINT stride = sizeof(VERTEX);
        UINT offset = 0;
        g_pDevcon->IASetVertexBuffers(0, 1, &g_pVBuffer, &stride, &offset);

        g_pDevcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        g_pDevcon->Draw(3, 0);
    }

    g_pSwapchain->Present(0, 0);
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
    hWnd = CreateWindowEx(0, _T("WindowClass1"), _T("Hello, Engine![Direct 3D]"),
                          WS_OVERLAPPEDWINDOW,
                          100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL,
                          hInstance, NULL);

    // Display the window in the mode specified by nCmdShow
    ShowWindow(hWnd, nCmdShow);

    MSG msg; // Message structure holding system messages retrieved from the queue

    // Message loop: GetMessage retrieves a message from the queue; returns 0 on WM_QUIT to exit the loop
    while (GetMessage(&msg, nullptr, 0, 0))
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
    LRESULT result = 0;
    bool wasHandled = false;

    switch (message)
    {
    case WM_CREATE:
        wasHandled = true;
        break;

    case WM_PAINT:
        result = CreateGraphicsResources(hWnd);
        RenderFrame();
        wasHandled = true;
        break;

    case WM_SIZE:
        if (g_pSwapchain != nullptr)
        {
            DiscardGraphicsResources();
        }
        wasHandled = true;
        break;

    case WM_DESTROY: // Received when the user clicks the close button and the window is about to be destroyed
        DiscardGraphicsResources();

        // Post WM_QUIT to the message queue so GetMessage above returns 0 and the message loop ends
        PostQuitMessage(0);

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
