#include <Windows.h>

#define STOLEN_BYTES_LEN 7

/*
 * Insert x86 JUMP at "victim" address, jumping to newFunction
 */
bool PatchVictim(BYTE* victim, BYTE* newFunction)
{
    uintptr_t relativeOffset;
    DWORD flOldProtect;
    DWORD unused;

    if (!VirtualProtect(victim, STOLEN_BYTES_LEN, PAGE_EXECUTE_READWRITE, &flOldProtect))
    {
        return false;
    }

    // Fill with NOP (0x90)
    memset(victim, 0x90, STOLEN_BYTES_LEN);
    // Jump is relative to PC.. currently start of "victim" function + 5, so we calculate a relative jump to newFunction
    // as follows:
    relativeOffset = (uintptr_t)(newFunction - victim - 5);

    // Overwrite first character with E9 (x86 for JMP)
    *victim = (BYTE)0xE9;
    *(uintptr_t*)(victim + 1) = (uintptr_t)relativeOffset;

    if (!VirtualProtect(victim, STOLEN_BYTES_LEN, flOldProtect, &unused))
    {
        return false;
    }

    (void)FlushInstructionCache(GetCurrentProcess(), victim, STOLEN_BYTES_LEN);

    return true;
}

BYTE* HookWithTrampoline(BYTE* victim, BYTE* newFunction)
{
#define TRAMPOLINE_LEN ((STOLEN_BYTES_LEN) + 5)

    // Store the original STOLEN_BYTES_LEN bytes in "trampoline", plus a jump back to victim+STOLEN_BYTES_LEN
    uintptr_t relativeOffset;
    BYTE* lpTrampoline = (BYTE*)VirtualAlloc(0, TRAMPOLINE_LEN, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (lpTrampoline == nullptr)
    {
        return nullptr;
    }

    // Put the bytes that will be overwritten in the trampoline
    memcpy(lpTrampoline, victim, STOLEN_BYTES_LEN);

    // Jump from the end of the trampoline back to the original function
    // Jump is relative to PC.. currently at lpTrampoline + 5, so we calculate a relative jump to "victim + 0" as
    // follows:
    relativeOffset = (uintptr_t)(victim - lpTrampoline - 5);

    // Insert E9 (x86 for JMP)
    *(&lpTrampoline[STOLEN_BYTES_LEN]) = (BYTE)0xE9;
    *(uintptr_t*)(&lpTrampoline[STOLEN_BYTES_LEN + 1]) = relativeOffset;
    (void)FlushInstructionCache(GetCurrentProcess(), lpTrampoline, TRAMPOLINE_LEN);

    // BytePatch the victim to jump to user-defined newFunction
    if (!PatchVictim(victim, newFunction))
    {
        VirtualFree(lpTrampoline, 0, MEM_RELEASE);
        return nullptr;
    }

    // Return the address of the trampoline, i.e. calling this will be like calling the original function
    return lpTrampoline;

#undef TRAMPOLINE_LEN
}
