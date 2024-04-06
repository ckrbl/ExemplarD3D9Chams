#include "D3D9Hook.h"

#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include <string>

#include "X86Hook.h"

typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                               UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
typedef HRESULT(WINAPI* tSetStreamSource)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber,
                                          IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);

tEndScene oEndScene = nullptr;
tDrawIndexedPrimitive oDrawIndexedPrimitive = nullptr;
tSetStreamSource oSetStreamSource = nullptr;

bool g_bShadersInitialized = false;
UINT g_uiStride = 0;

IDirect3DPixelShader9* g_shaderRed;
IDirect3DPixelShader9* g_shaderYellow;

bool CreateShader(IDirect3DDevice9* Device, IDirect3DPixelShader9** Shader, float Red, float Green, float Blue,
                  float Alpha)
{
    char Buffer[128];
    HRESULT hr;
    ID3DXBuffer* ShaderBuffer;

    sprintf_s(Buffer, "ps.1.1\ndef c0, %f, %f, %f, %f\nmov r0, c0", Red, Green, Blue, Alpha);
    hr = D3DXAssembleShader(Buffer, sizeof(Buffer), 0, 0, 0, &ShaderBuffer, 0);
    if (hr != D3D_OK)
    {
        MessageBox(nullptr, "D3DXAssembleShader failed", nullptr, MB_OK);
        return false;
    }

    hr = Device->CreatePixelShader((const DWORD*)ShaderBuffer->GetBufferPointer(), Shader);
    if (hr != D3D_OK)
    {
        MessageBox(nullptr, "CreatePixelShader failed", nullptr, MB_OK);
        return false;
    }

    return true;
}

HRESULT ApplyChams(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex,
                   UINT NumVertices, UINT StartIndex, UINT primCount, IDirect3DPixelShader9* shaderHidden,
                   IDirect3DPixelShader9* shaderShown)
{
    if (!g_bShadersInitialized)
    {
        return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);
    }

    // Disable Fog, depth test
    pDevice->SetRenderState(D3DRS_FOGENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    pDevice->SetPixelShader(shaderHidden);

    // Color the model, render what's hidden
    oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);

    // Reenable depth test, keep fog disabled
    pDevice->SetRenderState(D3DRS_FOGENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    pDevice->SetPixelShader(shaderShown);

    // Set a different color, render what's visible
    return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);
}

HRESULT APIENTRY xEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    if (!g_bShadersInitialized)
    {
        if (!CreateShader(pDevice, &g_shaderRed, 1.0f, 0.0f, 0.0f, 1.0f))
        {
            // Message printed in CreateShader
            goto end;
        }

        if (!CreateShader(pDevice, &g_shaderYellow, 1.0f, 1.0f, 0.0f, 1.0f))
        {
            // Message printed in CreateShader
            goto end;
        }

        g_bShadersInitialized = true;
    }

end:
    return oEndScene(pDevice);
}

HRESULT WINAPI xDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT primCount)
{
    if (g_uiStride == 40)
    {
        return ApplyChams(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount, g_shaderYellow,
                          g_shaderRed);
    }

    return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);
}

HRESULT WINAPI xSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData,
                                UINT OffsetInBytes, UINT Stride)
{
    if (StreamNumber == 0)
    {
        g_uiStride = Stride;
    }

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
