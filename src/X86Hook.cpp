#include <Windows.h>

#define JUMP_PATCH_LEN 7

/*
 * Patch the original(victim) opcodes with a jump to the newFunction
 */
bool PatchVictim(BYTE* victim, BYTE* newFunction)
{
    uintptr_t relativeOffset;
    DWORD flOldProtect;
    DWORD unused;

    if (!VirtualProtect(victim, JUMP_PATCH_LEN, PAGE_EXECUTE_READWRITE, &flOldProtect))
    {
        return false;
    }

    // Fill with NOP (0x90)
    memset(victim, 0x90, JUMP_PATCH_LEN);
    // Jump is relative to PC.. currently start of "victim" function + 5, so we calculate a relative jump to newFunction
    // as follows:
    relativeOffset = (uintptr_t)(newFunction - victim - 5);

    // Overwrite first character with E9 (x86 for JMP)
    *victim = (BYTE)0xE9;
    *(uintptr_t*)(victim + 1) = (uintptr_t)relativeOffset;

    if (!VirtualProtect(victim, JUMP_PATCH_LEN, flOldProtect, &unused))
    {
        return false;
    }

    (void)FlushInstructionCache(GetCurrentProcess(), victim, JUMP_PATCH_LEN);

    return true;
}

BYTE* HookWithTrampoline(BYTE* victim, BYTE* newFunction)
{
    // Store the original bytes in "trampoline", plus a jump back to the original function
    uintptr_t relativeOffset;
    BYTE* lpTrampoline = (BYTE*)VirtualAlloc(0, JUMP_PATCH_LEN + 5, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (lpTrampoline == nullptr)
    {
        return nullptr;
    }

    // Put the bytes that will be overwritten in the trampoline
    memcpy(lpTrampoline, victim, JUMP_PATCH_LEN);

    // Jump from the end of the trampoline back to the original function
    // Jump is relative to PC.. currently at lpTrampoline + 5, so we calculate a relative jump to "victim + 0" as
    // follows:
    relativeOffset = (uintptr_t)(victim - lpTrampoline - 5);

    // Insert E9 (x86 for JMP)
    *(&lpTrampoline[JUMP_PATCH_LEN]) = (BYTE)0xE9;
    *(uintptr_t*)(&lpTrampoline[JUMP_PATCH_LEN + 1]) = relativeOffset;

    // Actually patch the victim address
    if (!PatchVictim(victim, newFunction))
    {
        VirtualFree(lpTrampoline, 0, MEM_RELEASE);
        return nullptr;
    }

    // Return the address of the trampoline, i.e. calling this will be like calling the original function
    return lpTrampoline;
}
