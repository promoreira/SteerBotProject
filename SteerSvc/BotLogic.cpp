#include "BotLogic.h"
#include <iostream>

// Definição real da variável
std::vector<SlaveInstance> g_activeSlaves;

void UpdateSlaves(std::vector<SlaveInstance>& slaves) {
    for (auto& slave : slaves) {
        if (!slave.isConnected || slave.addrRemoteState == 0) continue;

        // Copia os dados da DLL (no Slave) para a memória local (no SteerSvc)
        // Usamos o endereço que capturamos com o GetRemoteExportAddress
        SlaveState tempState;
        if (ReadProcessMemory(slave.hProcess, (LPCVOID)slave.addrRemoteState, &tempState, sizeof(SlaveState), NULL)) {
            slave.localData = tempState;
        }
        else {
            // Se falhar a leitura, talvez o jogo fechou ou a DLL descarregou
            slave.isConnected = false;
        }
    }
}
