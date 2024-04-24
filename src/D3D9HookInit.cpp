#include "D3D9Hook.h"

#include "detours.h"

#pragma comment(lib, "detours.lib")

bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE])
{
#define RESET_VTABLE_IDX 16
#define ENDSCENE_VTABLE_IDX 42
#define DRAW_INDEXED_PRIMITIVE_VTABLE_IDX 82
#define SET_STREAM_SOURCE_VTABLE_IDX 100

    bool status = false;

    if (DetourTransactionBegin() != NO_ERROR)
    {
        MessageBox(nullptr, "DetourTransactionBegin failed", nullptr, MB_OK);
        goto decide;
    }

    if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR)
    {
        MessageBox(nullptr, "DetourTransactionBegin failed", nullptr, MB_OK);
        goto decide;
    }

    oReset = (tReset)(d3d9Device[RESET_VTABLE_IDX]);
    if (DetourAttach(&(LPVOID&)oReset, xReset) != NO_ERROR)
    {
        MessageBox(nullptr, "DetourAttach failed", nullptr, MB_OK);
        goto decide;
    }

    oEndScene = (tEndScene)(d3d9Device[ENDSCENE_VTABLE_IDX]);
    if (DetourAttach(&(LPVOID&)oEndScene, xEndScene) != NO_ERROR)
    {
        MessageBox(nullptr, "DetourAttach failed", nullptr, MB_OK);
        goto decide;
    }

    oDrawIndexedPrimitive = (tDrawIndexedPrimitive)(d3d9Device[DRAW_INDEXED_PRIMITIVE_VTABLE_IDX]);
    if (DetourAttach(&(LPVOID&)oDrawIndexedPrimitive, xDrawIndexedPrimitive) != NO_ERROR)
    {
        MessageBox(nullptr, "DetourAttach failed", nullptr, MB_OK);
        goto decide;
    }

    oSetStreamSource = (tSetStreamSource)(d3d9Device[SET_STREAM_SOURCE_VTABLE_IDX]);
    if (DetourAttach(&(LPVOID&)oSetStreamSource, xSetStreamSource) != NO_ERROR)
    {
        MessageBox(nullptr, "DetourAttach failed", nullptr, MB_OK);
        goto decide;
    }

    status = true;

decide:
    if (status)
    {
        if (DetourTransactionCommit() != NO_ERROR)
        {
            MessageBox(nullptr, "DetourTransactionCommit failed", nullptr, MB_OK);
            status = false;
            goto exit;
        }
    }
    else
    {
        if (DetourTransactionAbort() != NO_ERROR)
        {
            MessageBox(nullptr, "DetourTransactionAbort failed", nullptr, MB_OK);
            goto exit;
        }
    }

exit:
    return status;

#undef SET_STREAM_SOURCE_VTABLE_IDX
#undef DRAW_INDEXED_PRIMITIVE_VTABLE_IDX
#undef ENDSCENE_VTABLE_IDX
#undef RESET_VTABLE_IDX
}
