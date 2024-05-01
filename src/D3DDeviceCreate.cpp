#include "D3DDeviceCreate.hpp"

#include <D3dx9core.h>
#include <windows.h>

LRESULT CALLBACK MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool CreateDeviceD3DCopyTable(HWND hWnd, void** pTable, size_t tableSize)
{
    bool bRet = false;
    IDirect3DDevice9* pDummyDevice = nullptr;
    HRESULT hrCreateDevice;
    D3DPRESENT_PARAMETERS d3dpp = {0};
    IDirect3D9* pD3D;
    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D)
    {
        MessageBox(nullptr, "Direct3DCreate9 failed", nullptr, MB_OK);
        goto exit;
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
exit:
    return bRet;
}

bool GetD3D9Device(void** pTable, size_t tableSize)
{
    bool bRet = false;
    HWND hWnd = NULL;
    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     MsgProc,
                     0L,
                     0L,
                     GetModuleHandle(nullptr),
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr,
                     "DX",
                     nullptr};

    if (RegisterClassEx(&wc) == 0)
    {
        MessageBox(nullptr, "RegisterClassEx failed", nullptr, MB_OK);
        goto exit;
    }

    hWnd = CreateWindow(wc.lpszClassName, nullptr, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, nullptr, nullptr,
                        wc.hInstance, nullptr);
    if (!hWnd)
    {
        MessageBox(nullptr, "CreateWindow failed", nullptr, MB_OK);
        goto cleanup_register_class;
    }

    if (!CreateDeviceD3DCopyTable(hWnd, pTable, tableSize))
    {
        // Message already printed
        goto cleanup_destroy_window;
    }

    bRet = true;

cleanup_destroy_window:
    DestroyWindow(hWnd);
cleanup_register_class:
    UnregisterClass(wc.lpszClassName, wc.hInstance);
exit:
    return bRet;
}
