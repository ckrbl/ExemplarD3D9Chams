#include <windows.h>
#include <d3d9.h>
#include <D3dx9core.h>
#include <string>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
