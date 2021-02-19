#include "stdafx.h"
#include "hook.h"

// EXAMPLE
static decltype(GetCurrentDirectoryW)* g_getCurrentDirectory = nullptr;

DWORD
WINAPI
GetCurrentDirectoryWHook(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return +1) LPWSTR lpBuffer
) {
    LOG_DEBUG << "GetCurrentDirectoryW is called";
    return g_getCurrentDirectory(nBufferLength, lpBuffer);
}
// END OF EXAMPLE

void HookAll() {
    g_getCurrentDirectory = Hook(L"kernel32.dll", "GetCurrentDirectoryW", GetCurrentDirectoryWHook);
    // Add your own hooks here
}
