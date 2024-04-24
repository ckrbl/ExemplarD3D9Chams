#include <Windows.h>
#include <string>

#include "D3D9Hook.h"

tReset oReset = nullptr;
tEndScene oEndScene = nullptr;
tDrawIndexedPrimitive oDrawIndexedPrimitive = nullptr;
tSetStreamSource oSetStreamSource = nullptr;

bool g_bShadersInitialized = false;
UINT g_uiStride = 0;

ShaderAndBuffer g_shaderRed;
ShaderAndBuffer g_shaderYellow;

bool CreateShader(IDirect3DDevice9* Device, ShaderAndBuffer& Shader, float Red, float Green, float Blue,
                  float Alpha)
{
    char Buffer[128];
    HRESULT hr;

    sprintf_s(Buffer, "ps.1.1\ndef c0, %f, %f, %f, %f\nmov r0, c0", Red, Green, Blue, Alpha);
    hr = D3DXAssembleShader(Buffer, sizeof(Buffer), 0, 0, 0, &Shader.dxBuffer, 0);
    if (hr != D3D_OK)
    {
        MessageBox(nullptr, "D3DXAssembleShader failed", nullptr, MB_OK);
        return false;
    }

    hr = Device->CreatePixelShader((const DWORD*)Shader.dxBuffer->GetBufferPointer(), &Shader.pixelShader);
    if (hr != D3D_OK)
    {
        MessageBox(nullptr, "CreatePixelShader failed", nullptr, MB_OK);
        return false;
    }

    return true;
}

HRESULT ApplyChams(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex,
                   UINT NumVertices, UINT StartIndex, UINT primCount, ShaderAndBuffer& shaderHidden,
                   ShaderAndBuffer& shaderShown)
{
    if (!g_bShadersInitialized)
    {
        return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);
    }

    // Disable Fog, depth test
    pDevice->SetRenderState(D3DRS_FOGENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    pDevice->SetPixelShader(shaderHidden.pixelShader);

    // Color the model, render what's hidden
    oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);

    // Reenable depth test, keep fog disabled
    pDevice->SetRenderState(D3DRS_FOGENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    pDevice->SetPixelShader(shaderShown.pixelShader);

    // Set a different color, render what's visible
    return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, primCount);
}

void PreReset()
{
#define RELEASE_SHADER_AND_BUFFER(x) \
    x.pixelShader->Release(); \
    x.dxBuffer->Release();

    if (!g_bShadersInitialized)
    {
        return;
    }

    RELEASE_SHADER_AND_BUFFER(g_shaderRed);
    RELEASE_SHADER_AND_BUFFER(g_shaderYellow);
    g_bShadersInitialized = false;

#undef RELEASE_SHADER_AND_BUFFER
}

void PostReset(LPDIRECT3DDEVICE9 pDevice)
{
    if (!CreateShader(pDevice, g_shaderRed, 1.0f, 0.0f, 0.0f, 1.0f))
    {
        // Message printed in CreateShader
        return;
    }

    if (!CreateShader(pDevice, g_shaderYellow, 1.0f, 1.0f, 0.0f, 1.0f))
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
