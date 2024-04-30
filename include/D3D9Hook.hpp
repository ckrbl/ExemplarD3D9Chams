#pragma once

#include <D3dx9core.h>
#include <Windows.h>
#include <d3d9.h>

#define D3D9_VTABLE_SIZE 119

typedef struct ShaderAndBuffer
{
    IDirect3DPixelShader9* pixelShader;
    ID3DXBuffer* dxBuffer;
} ShaderAndBuffer;

typedef HRESULT(WINAPI* tReset)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                               UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
typedef HRESULT(WINAPI* tSetStreamSource)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber,
                                          IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);

extern tReset oReset;
extern tEndScene oEndScene;
extern tDrawIndexedPrimitive oDrawIndexedPrimitive;
extern tSetStreamSource oSetStreamSource;

HRESULT WINAPI xReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT APIENTRY xEndScene(LPDIRECT3DDEVICE9 pDevice);
HRESULT WINAPI xDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT primCount);
HRESULT WINAPI xSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData,
                                UINT OffsetInBytes, UINT Stride);

bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE]);
