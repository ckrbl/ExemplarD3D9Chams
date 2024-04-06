#pragma once

#define D3D9_VTABLE_SIZE 119

bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE]);
