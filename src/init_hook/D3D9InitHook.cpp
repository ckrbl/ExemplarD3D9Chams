#include "D3D9InitHook.hpp"

#include "D3D9Hook.hpp"
#include "detours.h"

#pragma comment(lib, "detours.lib")

#define STRINGIFY(x) #x

#define CHECK_ERROR(FUNC, ERROR_TARGET)                                 \
    if ((FUNC) != NO_ERROR)                                             \
    {                                                                   \
        MessageBox(nullptr, STRINGIFY(FUNC) " failed", nullptr, MB_OK); \
        status = false;                                                 \
        goto ERROR_TARGET;                                              \
    }

#define DETOUR_ATTACH(NAME, TABLE_IDX)          \
    o##NAME = (t##NAME)(d3d9Device[TABLE_IDX]); \
    CHECK_ERROR(DetourAttach(&(LPVOID&)o##NAME, x##NAME), decide);

// clang-format off
#define DETOUR_DETACH(NAME) \
    CHECK_ERROR(DetourDetach(&(LPVOID&)o##NAME, x##NAME), decide);
// clang-format on

bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE])
{
#define ADDREF_VTABLE_IDX 1
#define RELEASE_VTABLE_IDX 2
#define RESET_VTABLE_IDX 16
#define ENDSCENE_VTABLE_IDX 42
#define DRAW_INDEXED_PRIMITIVE_VTABLE_IDX 82
#define SET_STREAM_SOURCE_VTABLE_IDX 100

    bool status;

    CHECK_ERROR(DetourTransactionBegin(), decide);
    CHECK_ERROR(DetourUpdateThread(GetCurrentThread()), decide);

    DETOUR_ATTACH(AddRef, ADDREF_VTABLE_IDX);
    DETOUR_ATTACH(Release, RELEASE_VTABLE_IDX);
    DETOUR_ATTACH(Reset, RESET_VTABLE_IDX);
    DETOUR_ATTACH(EndScene, ENDSCENE_VTABLE_IDX);
    DETOUR_ATTACH(DrawIndexedPrimitive, DRAW_INDEXED_PRIMITIVE_VTABLE_IDX);
    DETOUR_ATTACH(SetStreamSource, SET_STREAM_SOURCE_VTABLE_IDX);

    status = true;

decide:
    if (status)
    {
        CHECK_ERROR(DetourTransactionCommit(), exit);
    }
    else
    {
        CHECK_ERROR(DetourTransactionAbort(), exit);
    }

exit:
    return status;

#undef SET_STREAM_SOURCE_VTABLE_IDX
#undef DRAW_INDEXED_PRIMITIVE_VTABLE_IDX
#undef ENDSCENE_VTABLE_IDX
#undef RESET_VTABLE_IDX
}

bool ShutdownD3D9Hook()
{
    int status;

    PreShutdown();
    CHECK_ERROR(DetourTransactionBegin(), decide);
    CHECK_ERROR(DetourUpdateThread(GetCurrentThread()), decide);

    DETOUR_DETACH(AddRef);
    DETOUR_DETACH(Release);
    DETOUR_DETACH(Reset);
    DETOUR_DETACH(EndScene);
    DETOUR_DETACH(DrawIndexedPrimitive);
    DETOUR_DETACH(SetStreamSource);

    status = true;

decide:
    if (status)
    {
        CHECK_ERROR(DetourTransactionCommit(), exit);
    }
    else
    {
        CHECK_ERROR(DetourTransactionAbort(), exit);
    }

exit:
    return status;
}

#undef DETOUR_ATTACH
#undef CHECK_ERROR
#undef STRINGIFY
