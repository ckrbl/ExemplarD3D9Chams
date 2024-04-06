#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include <string>

#include "D3DDeviceCreate.h"
#include "X86Hook.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

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
