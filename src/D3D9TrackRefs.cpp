#include <Windows.h>

#include <string>

#include "D3D9Hook.hpp"

tAddRef oAddRef = nullptr;
tRelease oRelease = nullptr;
tReset oReset = nullptr;

IDirect3DDevice9* g_pGameDevice = nullptr;
ULONG g_ulGameRefCount = 0;
ULONG g_ulMyAllocatedObjectsCount = 0;

#define RECORD_REFCOUNT_AND_RETURN_IT(CALL) \
    g_ulGameRefCount = CALL(pDevice);       \
    return g_ulGameRefCount;

ULONG WINAPI xAddRef(LPDIRECT3DDEVICE9 pDevice)
{
    // If IUnknown::AddRef is being called for some other object, ignore it
    if (pDevice != g_pGameDevice)
    {
        return oAddRef(pDevice);
    }

    // Write down what the current refcount is for the game's Direct3DDevice
    RECORD_REFCOUNT_AND_RETURN_IT(oAddRef);
}

ULONG WINAPI xRelease(LPDIRECT3DDEVICE9 pDevice)
{
    // If IUnknown::Release is being called for some other object, ignore it
    if (pDevice != g_pGameDevice)
    {
        return oRelease(pDevice);
    }

    // The object being released IS the game's Direct3DDevice.  Check if this is the final decement (when decremented
    // the only remaining refs will be ours), likely the game is shutting down
    // Not a great way to do this because if there's other stuff injected like overwolf or steam that alloctes its own
    // objects this probably won't work
    if ((g_ulGameRefCount - 1) == g_ulMyAllocatedObjectsCount)
    {
        PreShutdown();
    }

    RECORD_REFCOUNT_AND_RETURN_IT(oRelease);
}
