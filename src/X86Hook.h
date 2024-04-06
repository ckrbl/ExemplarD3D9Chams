#pragma once

#include <Windows.h>

BYTE* HookWithTrampoline(BYTE* victim, BYTE* newFunction);
