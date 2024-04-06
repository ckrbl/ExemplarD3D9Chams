#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include <string>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

DWORD WINAPI Init(LPVOID lpThreadParameter)
{
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
