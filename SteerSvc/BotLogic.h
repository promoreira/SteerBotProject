#pragma once
#include <windows.h>
#include <vector>
#include "SharedData.h"
#include "SteerPilot.h"

// Offset do Anti-AFK conforme seu script do CE
#define ANTI_AFK_OFFSET 0x9F75E1 

struct SlaveInstance {
    DWORD pid;
    HWND hwnd;
    HANDLE hProcess;
    uintptr_t addrRemoteState;
    SlaveState localData;
    bool isConnected;

    // O objeto pilot vive aqui para lembrar a direção anterior (Auto-Aprendizado)
    SteerPilot pilot;
};

extern std::vector<SlaveInstance> g_activeSlaves;

void UpdateSlaves(std::vector<SlaveInstance>& slaves);
uintptr_t GetModuleBase(DWORD pid, const char* modName);
void PatchAntiAfk(HANDLE hProc, uintptr_t baseAddr, bool active);
