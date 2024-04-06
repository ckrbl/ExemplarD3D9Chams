#include "D3D9Hook.h"

#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include "X86Hook.h"

typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                               UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
typedef HRESULT(WINAPI* tSetStreamSource)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber,
                                          IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);

tEndScene oEndScene = nullptr;
tDrawIndexedPrimitive oDrawIndexedPrimitive = nullptr;
tSetStreamSource oSetStreamSource = nullptr;

HRESULT APIENTRY xEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    return oEndScene(pDevice);
}

HRESULT WINAPI xDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT primCount)
{
    return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);
}

HRESULT WINAPI xSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData,
                                UINT OffsetInBytes, UINT Stride)
{
    return oSetStreamSource(pDevice, StreamNumber, pStreamData, OffsetInBytes, Stride);
}

bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE])
{
#define ENDSCENE_VTABLE_IDX 42
#define DRAW_INDEXED_PRIMITIVE_VTABLE_IDX 82
#define SET_STREAM_SOURCE_VTABLE_IDX 100

    oEndScene = (tEndScene)HookWithTrampoline((BYTE*)d3d9Device[ENDSCENE_VTABLE_IDX], (BYTE*)xEndScene);
    if (!oEndScene)
    {
        MessageBox(nullptr, "Endscene hook failed", nullptr, MB_OK);
        return false;
    }

    oDrawIndexedPrimitive = (tDrawIndexedPrimitive)HookWithTrampoline(
        (BYTE*)d3d9Device[DRAW_INDEXED_PRIMITIVE_VTABLE_IDX], (BYTE*)xDrawIndexedPrimitive);
    if (!oDrawIndexedPrimitive)
    {
        MessageBox(nullptr, "DrawIndexedPrimitive hook failed", nullptr, MB_OK);
        return false;
    }

    oSetStreamSource =
        (tSetStreamSource)HookWithTrampoline((BYTE*)d3d9Device[SET_STREAM_SOURCE_VTABLE_IDX], (BYTE*)xSetStreamSource);
    if (!oSetStreamSource)
    {
        MessageBox(nullptr, "SetStreamSource hook failed", nullptr, MB_OK);
        return false;
    }

    return true;

#undef SET_STREAM_SOURCE_VTABLE_IDX
#undef DRAW_INDEXED_PRIMITIVE_VTABLE_IDX
#undef ENDSCENE_VTABLE_IDX
}
