#include "D3D9Hook.hpp"
#include "detours.h"

#pragma comment(lib, "detours.lib")

bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE])
{
#define RESET_VTABLE_IDX 16
#define ENDSCENE_VTABLE_IDX 42
#define DRAW_INDEXED_PRIMITIVE_VTABLE_IDX 82
#define SET_STREAM_SOURCE_VTABLE_IDX 100

#define DETOUR_ATTACH(NAME, TABLE_IDX)                              \
    o##NAME = (t##NAME)(d3d9Device[TABLE_IDX]);                     \
    if (DetourAttach(&(LPVOID&)o##NAME, x##NAME) != NO_ERROR)       \
    {                                                               \
        MessageBox(nullptr, "DetourAttach failed", nullptr, MB_OK); \
        goto decide;                                                \
    }

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

    DETOUR_ATTACH(Reset, RESET_VTABLE_IDX)
    DETOUR_ATTACH(EndScene, ENDSCENE_VTABLE_IDX)
    DETOUR_ATTACH(DrawIndexedPrimitive, DRAW_INDEXED_PRIMITIVE_VTABLE_IDX)
    DETOUR_ATTACH(SetStreamSource, SET_STREAM_SOURCE_VTABLE_IDX)

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

#undef DETOUR_ATTACH

#undef SET_STREAM_SOURCE_VTABLE_IDX
#undef DRAW_INDEXED_PRIMITIVE_VTABLE_IDX
#undef ENDSCENE_VTABLE_IDX
#undef RESET_VTABLE_IDX
}
