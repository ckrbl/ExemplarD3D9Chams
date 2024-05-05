#pragma once

#define D3D9_VTABLE_SIZE 119

bool CreateD3D9Device(void** pTable, size_t tableSize);
bool InitD3D9Hook(void* d3d9Device[D3D9_VTABLE_SIZE]);
bool ShutdownD3D9Hook();
