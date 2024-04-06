#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include <string>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

LRESULT CALLBACK MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool GetD3D9Device(void** pTable, size_t tableSize)
{
    bool bRet = false;
    D3DPRESENT_PARAMETERS d3dpp = {0};
    IDirect3DDevice9* pDummyDevice = NULL;
    HRESULT hrCreateDevice;
    HWND hWnd;
    IDirect3D9* pD3D;
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "DX", NULL};

    if (!pTable)
    {
        MessageBox(nullptr, "Error arguments", nullptr, MB_OK);
        goto error_exit;
    }

    (void)RegisterClassEx(&wc);
    hWnd =
        CreateWindow("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, wc.hInstance, NULL);
    if (!hWnd)
    {
        MessageBox(nullptr, "CreateWindow failed", nullptr, MB_OK);
        goto error_exit;
    }

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D)
    {
        MessageBox(nullptr, "Direct3DCreate9 failed", nullptr, MB_OK);
        goto cleanup_destroy_window;
    }

    d3dpp.Windowed = true;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    hrCreateDevice = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                        &d3dpp, &pDummyDevice);
    if (hrCreateDevice != S_OK)
    {
        MessageBox(nullptr, "CreateDevice failed", nullptr, MB_OK);
        goto cleanup_free_pd3d;
    }

    memcpy(pTable, *reinterpret_cast<void***>(pDummyDevice), tableSize);
    bRet = true;

    pDummyDevice->Release();
cleanup_free_pd3d:
    pD3D->Release();
cleanup_destroy_window:
    DestroyWindow(hWnd);
error_exit:
    return bRet;
}

DWORD WINAPI Init(LPVOID lpThreadParameter)
{
    void* d3d9Device[119];
    if (!GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
    {
        MessageBox(nullptr, "GetD3D9Device failed", nullptr, MB_OK);
        return -1;
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    HANDLE hThread;

    // Disable DLL_THREAD_ATTACH as we're about to create a thread and are holding Loader Lock
    DisableThreadLibraryCalls(hinstDLL);

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            hThread = CreateThread(nullptr, 0, Init, nullptr, 0, nullptr);
            if (hThread == nullptr)
            {
                return FALSE;
            }

            CloseHandle(hThread);
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
