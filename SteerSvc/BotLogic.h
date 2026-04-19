#pragma once
#include <vector>
#include "SharedData.h"
#include <windows.h>

// A struct precisa estar visível aqui. Se ela estiver no Main.cpp, 
// o ideal é movê-la para um arquivo como SlaveInstance.h para todos enxergarem.
struct SlaveInstance {
    DWORD pid;
    HWND hwnd;
    HANDLE hProcess;
    uintptr_t addrRemoteState; // Endereço de onde os dados estão no Slave
    SlaveState localData;      // Cópia local dos dados para o OpenSteer usar
    bool isConnected;
};

// Primeiro a função que manipula a lista
void UpdateSlaves(std::vector<SlaveInstance>& slaves);

// E por último o "aviso" de que a lista global existe
extern std::vector<SlaveInstance> g_activeSlaves;
