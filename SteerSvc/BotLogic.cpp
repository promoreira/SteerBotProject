#include "BotLogic.h"
#include <TlHelp32.h>

std::vector<SlaveInstance> g_activeSlaves;
static BYTE s_originalAntiAfk[8] = { 0 };
static bool s_antiAfkBytesSaved = false;

uintptr_t GetModuleBase(DWORD pid, const char* modName) {
    uintptr_t addr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me; me.dwSize = sizeof(me);
        if (Module32First(hSnap, &me)) {
            do {
                if (_stricmp(me.szModule, modName) == 0) {
                    addr = (uintptr_t)me.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &me));
        }
        CloseHandle(hSnap);
    }
    return addr;
}

void UpdateSlaves(std::vector<SlaveInstance>& slaves) {
    for (auto& s : slaves) {
        if (s.isConnected && s.addrRemoteState != 0) {
            ReadProcessMemory(s.hProcess, (LPCVOID)s.addrRemoteState, &s.localData, sizeof(SlaveState), NULL);
        }
    }
}

void PatchAntiAfk(HANDLE hProc, uintptr_t baseAddr, bool active) {
    if (!hProc || !baseAddr) return;
    uintptr_t target = baseAddr + ANTI_AFK_OFFSET;

    if (!s_antiAfkBytesSaved) {
        ReadProcessMemory(hProc, (LPCVOID)target, s_originalAntiAfk, 8, NULL);
        s_antiAfkBytesSaved = true;
    }

    DWORD old;
    VirtualProtectEx(hProc, (LPVOID)target, 8, PAGE_EXECUTE_READWRITE, &old);
    if (active) {
        // NOPs para anular a instrução de incremento do timer AFK
        BYTE nopPatch[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        WriteProcessMemory(hProc, (LPVOID)target, nopPatch, 8, NULL);
    }
    else {
        WriteProcessMemory(hProc, (LPVOID)target, s_originalAntiAfk, 8, NULL);
    }
    VirtualProtectEx(hProc, (LPVOID)target, 8, old, &old);
}
