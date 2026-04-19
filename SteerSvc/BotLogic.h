#pragma once
#include <windows.h>
#include <vector>
#include "SharedData.h"
#include "SteerPilot.h"

// Offset do Anti-AFK
#define ANTI_AFK_OFFSET 0x9F75E1 

struct SlaveInstance {
    DWORD pid;
    HWND hwnd;
    HANDLE hProcess;
    uintptr_t addrRemoteState;
    SlaveState localData;
    bool isConnected;

    SteerPilot pilot; // Persistência para o cálculo de direção
};

// Variáveis Globais
extern std::vector<SlaveInstance> g_activeSlaves;

// Protótipos
void UpdateSlaves(std::vector<SlaveInstance>& slaves);
uintptr_t GetModuleBase(DWORD pid, const char* modName);
void PatchAntiAfk(HANDLE hProc, uintptr_t baseAddr, bool active);
