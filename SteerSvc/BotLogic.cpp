#include "BotLogic.h"
#include <TlHelp32.h>

std::vector<SlaveInstance> g_activeSlaves;

// Variáveis de controle para o Anti-AFK
static BYTE s_originalAntiAfk[8] = { 0 };
static bool s_antiAfkBytesSaved = false;

uintptr_t GetModuleBase(DWORD pid, const char* modName) {
    uintptr_t addr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me;
        me.dwSize = sizeof(me);
        if (Module32First(hSnap, &me)) {
            do {
                if (strcmp(me.szModule, modName) == 0) {
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
    if (hProc == NULL || baseAddr == 0) return;
    uintptr_t target = baseAddr + ANTI_AFK_OFFSET;

    // Bytes originais conforme o script do CE (8 bytes)
    static BYTE originalBytes[8] = { 0xF3, 0x0F, 0x58, 0x05, 0x1B, 0xD7, 0x70, 0x01 };

    DWORD oldProtect;
    VirtualProtectEx(hProc, (LPVOID)target, 8, PAGE_EXECUTE_READWRITE, &oldProtect);

    if (active) {
        // NOPando a instrução (0x90 = NOP). 
        // Isso impede que o timer de AFK seja somado/atualizado.
        BYTE nopPatch[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        WriteProcessMemory(hProc, (LPVOID)target, nopPatch, 8, NULL);
    }
    else {
        // Restaura os bytes originais do DCGAME.EXE
        WriteProcessMemory(hProc, (LPVOID)target, originalBytes, 8, NULL);
    }

    VirtualProtectEx(hProc, (LPVOID)target, 8, oldProtect, &oldProtect);
}
