#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include <string>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

HWND g_ProcessWindow;

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcId;
    GetWindowThreadProcessId(handle, &wndProcId);

    if (GetCurrentProcessId() != wndProcId)
        return TRUE;  // skip to next window

    g_ProcessWindow = handle;
    return FALSE;  // window found abort search
}

HWND GetProcessWindow()
{
    g_ProcessWindow = NULL;
    EnumWindows(EnumWindowsCallback, NULL);
    return g_ProcessWindow;
}

bool GetD3D9Device(void** pTable, size_t Size)
{
    if (!pTable)
        return false;

    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

    if (!pD3D)
        return false;

    IDirect3DDevice9* pDummyDevice = NULL;

    // options to create dummy device
    D3DPRESENT_PARAMETERS presentParams = {};
    presentParams.Windowed = false;
    presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    presentParams.hDeviceWindow = GetProcessWindow();

    HRESULT dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, presentParams.hDeviceWindow,
                                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pDummyDevice);

    if (dummyDeviceCreated != S_OK)
    {
        // may fail in windowed fullscreen mode, trying again with windowed mode
        presentParams.Windowed = true;

        dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, presentParams.hDeviceWindow,
                                                D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pDummyDevice);

        if (dummyDeviceCreated != S_OK)
        {
            pD3D->Release();
            return false;
        }
    }

    memcpy(pTable, *reinterpret_cast<void***>(pDummyDevice), Size);

    pDummyDevice->Release();
    pD3D->Release();
    return true;
}

DWORD WINAPI Init(LPVOID lpThreadParameter)
{
    void* d3d9Device[119];
    if (!GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
    {
        MessageBox(nullptr, "GetD3D9Device failed", "err", MB_OK);
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
