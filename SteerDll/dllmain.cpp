#include <windows.h>
#include <psapi.h>
#include <vector>
#include <stdio.h>
#include "MinHook.h"
#include "SharedData.h"

extern "C" {
    __declspec(dllexport) SlaveState g_SlaveState = { {0.0f, 0.0f, 0.0f}, 0.0f, false, 0 };
    uintptr_t _playerbase = 0;
    uintptr_t fpReturnAddress = 0;
}

extern "C" void hook_capture_pos();

uintptr_t FindPattern(const char* module, const char* pattern) {
    uintptr_t base = (uintptr_t)GetModuleHandleA(module);
    if (!base) return 0;
    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), (HMODULE)base, &modInfo, sizeof(MODULEINFO));
    uintptr_t size = (uintptr_t)modInfo.SizeOfImage;
    auto patternToBytes = [](const char* pattern) {
        std::vector<int> bytes;
        char* start = const_cast<char*>(pattern);
        char* end = const_cast<char*>(pattern) + strlen(pattern);
        for (char* cur = start; cur < end; ++cur) {
            if (*cur == '?') { bytes.push_back(-1); if (*(cur + 1) == '?') ++cur; }
            else { bytes.push_back((int)strtol(cur, &cur, 16)); }
        }
        return bytes;
        };
    std::vector<int> patternBytes = patternToBytes(pattern);
    unsigned char* scanStart = (unsigned char*)base;
    for (uintptr_t i = 0; i < size - patternBytes.size(); ++i) {
        bool found = true;
        for (uintptr_t j = 0; j < patternBytes.size(); ++j) {
            if (patternBytes[j] != -1 && scanStart[i + j] != (unsigned char)patternBytes[j]) { found = false; break; }
        }
        if (found) return (uintptr_t)&scanStart[i];
    }
    return 0;
}

void SetupHooks() {
    if (MH_Initialize() != MH_OK) return;

    // Usamos o seu AOB que encontrou o player perfeitamente
    uintptr_t targetAddr = FindPattern("DCGAME.EXE", "F3 0F 10 50 08 F3 0F 10 48 04 F3 0F 10 00");

    if (targetAddr != 0) {
        // Hookamos no início da sequência de 14 bytes
        uintptr_t hookPoint = targetAddr;
        fpReturnAddress = hookPoint + 14; // O pulo de volta é após os 14 bytes

        // O MinHook agora vai usar o nosso ASM para gerenciar o salto
        MH_CreateHook((LPVOID)hookPoint, &hook_capture_pos, NULL);
        MH_EnableHook(MH_ALL_HOOKS);
    }
}

DWORD WINAPI UpdateThread(LPVOID lpParam) {
    while (true) {
        if (_playerbase > 0x10000) {
            g_SlaveState.isAlive = true;
            g_SlaveState.pBase = _playerbase;
            g_SlaveState.currentPos.x = *(float*)(_playerbase + 0x00);
            g_SlaveState.currentPos.y = *(float*)(_playerbase + 0x04);
            g_SlaveState.currentPos.z = *(float*)(_playerbase + 0x08);
            g_SlaveState.currentYaw = *(float*)(_playerbase + 0x24);
        }
        Sleep(10);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        SetupHooks();
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateThread, NULL, 0, NULL);
    }
    return TRUE;
}
