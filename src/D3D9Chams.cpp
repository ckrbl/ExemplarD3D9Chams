#include <Windows.h>

#include <string>

#include "D3D9Hook.hpp"

bool g_bShutdownStarted = false;
bool g_bShadersInitialized = false;
UINT g_uiStride = 0;

tEndScene oEndScene = nullptr;
tDrawIndexedPrimitive oDrawIndexedPrimitive = nullptr;
tSetStreamSource oSetStreamSource = nullptr;

IDirect3DPixelShader9* g_shaderRed;
IDirect3DPixelShader9* g_shaderYellow;

static inline bool CreateShader(IDirect3DDevice9* Device, IDirect3DPixelShader9** Shader, float Red, float Green,
                                float Blue, float Alpha)
{
    char cBuffer[128];
    ID3DXBuffer* pShaderBuffer = NULL;
    HRESULT hr;

    sprintf_s(cBuffer, "ps.1.1\ndef c0, %f, %f, %f, %f\nmov r0, c0", Red, Green, Blue, Alpha);
    hr = D3DXAssembleShader(cBuffer, sizeof(cBuffer), 0, 0, 0, &pShaderBuffer, 0);
    if (hr != D3D_OK)
    {
        MessageBox(nullptr, "D3DXAssembleShader failed", nullptr, MB_OK);
        return false;
    }

    hr = Device->CreatePixelShader((const DWORD*)pShaderBuffer->GetBufferPointer(), Shader);
    if (hr != D3D_OK)
    {
        MessageBox(nullptr, "CreatePixelShader failed", nullptr, MB_OK);
        return false;
    }

    // Calling Device->CreatePixelShader will cause the refcount of the game's Direct3DDevice to increase. We need to
    // track this
    g_ulMyAllocatedObjectsCount++;
    return true;
}

static inline void DestroyShader(IDirect3DPixelShader9* Shader)
{
    if (Shader)
    {
        g_ulMyAllocatedObjectsCount--;
    }

    SAFE_RELEASE(Shader);
}

void PreReset()
{
    if (!g_bShadersInitialized)
    {
        return;
    }

    DestroyShader(g_shaderRed);
    DestroyShader(g_shaderYellow);
    g_bShadersInitialized = false;
}

void PostReset(LPDIRECT3DDEVICE9 pDevice)
{
    g_pGameDevice = pDevice;
    if (g_bShadersInitialized || g_bShutdownStarted)
    {
        return;
    }

    if (!CreateShader(pDevice, &g_shaderRed, 1.0f, 0.0f, 0.0f, 1.0f))
    {
        // Message printed in CreateShader
        return;
    }

    if (!CreateShader(pDevice, &g_shaderYellow, 1.0f, 1.0f, 0.0f, 1.0f))
    {
        // Message printed in CreateShader
        DestroyShader(g_shaderRed);
        return;
    }

    MemoryBarrier();
    g_bShadersInitialized = true;
}

void PreShutdown()
{
    g_bShutdownStarted = true;
    MemoryBarrier();
    PreReset();
}

static inline HRESULT ApplyChams(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex,
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
    PostReset(pDevice);
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

HRESULT WINAPI xReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    PreReset();
    HRESULT hrReset = oReset(pDevice, pPresentationParameters);
    PostReset(pDevice);
    return hrReset;
}
