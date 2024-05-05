#pragma once

#include <D3dx9core.h>
#include <Windows.h>
#include <d3d9.h>

#define SAFE_RELEASE(d) \
    if (d)              \
    {                   \
        d->Release();   \
        d = NULL;       \
    }

typedef struct ShaderAndBuffer
{
    IDirect3DPixelShader9* pixelShader;
    ID3DXBuffer* dxBuffer;
} ShaderAndBuffer;

extern IDirect3DDevice9* g_pGameDevice;
extern ULONG g_ulMyAllocatedObjectsCount;
extern bool g_bShutdownStarted;
void PreShutdown();
inline void PreReset();
inline void PostReset(LPDIRECT3DDEVICE9 pDevice);

typedef ULONG(WINAPI* tAddRef)(LPDIRECT3DDEVICE9 pDevice);
typedef ULONG(WINAPI* tRelease)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(WINAPI* tReset)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                               UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
typedef HRESULT(WINAPI* tSetStreamSource)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber,
                                          IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);

extern tAddRef oAddRef;
extern tRelease oRelease;
extern tReset oReset;
extern tEndScene oEndScene;
extern tDrawIndexedPrimitive oDrawIndexedPrimitive;
extern tSetStreamSource oSetStreamSource;

ULONG WINAPI xAddRef(LPDIRECT3DDEVICE9 pDevice);
ULONG WINAPI xRelease(LPDIRECT3DDEVICE9 pDevice);
HRESULT WINAPI xReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT APIENTRY xEndScene(LPDIRECT3DDEVICE9 pDevice);
HRESULT WINAPI xDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT primCount);
HRESULT WINAPI xSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData,
                                UINT OffsetInBytes, UINT Stride);
