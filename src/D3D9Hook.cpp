#include "D3D9Hook.h"

#include <D3dx9core.h>
#include <d3d9.h>
#include <windows.h>

#include <string>

#include "detours.h"

#pragma comment(lib, "detours.lib")

typedef HRESULT(WINAPI* tReset)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef HRESULT(APIENTRY* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                               UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
typedef HRESULT(WINAPI* tSetStreamSource)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber,
                                          IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);

tReset oReset = nullptr;
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

void PreReset()
{
    if (!g_bShadersInitialized)
    {
        return;
    }

    g_shaderRed->Release();
    g_shaderYellow->Release();
    g_bShadersInitialized = false;
}

void PostReset(LPDIRECT3DDEVICE9 pDevice)
{
    if (!CreateShader(pDevice, &g_shaderRed, 1.0f, 0.0f, 0.0f, 1.0f))
    {
        // Message printed in CreateShader
        return;
    }

    if (!CreateShader(pDevice, &g_shaderYellow, 1.0f, 1.0f, 0.0f, 1.0f))
    {
        // Message printed in CreateShader
        return;
    }

    g_bShadersInitialized = true;
}

HRESULT WINAPI xReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    PreReset();
    return oReset(pDevice, pPresentationParameters);
}

HRESULT APIENTRY xEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    if (!g_bShadersInitialized)
    {
        PostReset(pDevice);
    }

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
