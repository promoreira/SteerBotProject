#pragma once
#include <windows.h>

// Estrutura básica para coordenadas 3D
struct Vec3 {
    float x, y, z;
};

// Estrutura de dados que a DLL exporta para o Injector ler
// O alinhamento deve ser idêntico em ambos os projetos
#pragma pack(push, 1)
struct SlaveState {
    Vec3 currentPos;    // Posição X, Y, Z capturada do jogo
    float currentYaw;   // Rotação (para onde o boneco olha)
    bool isAlive;       // Status se o player foi encontrado
    uintptr_t pBase;    // Endereço real (RAX) capturado pelo Hook ASM
};
#pragma pack(pop)

// Nome da variável que o GetProcAddress vai procurar na DLL
#define EXPORT_VAR_NAME "g_SlaveState"
