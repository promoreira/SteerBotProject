#pragma once
#include <windows.h>
#include <cmath>
#include "SharedData.h"

// Definições de Teclas (Caso não estejam no header do sistema)
#ifndef VK_W
#define VK_W 0x57
#endif
#ifndef VK_A
#define VK_A 0x41
#endif
#ifndef VK_S
#define VK_S 0x53
#endif
#ifndef VK_D
#define VK_D 0x44
#endif

class SteerPilot {
public:
    // Calcula o ângulo necessário para o Slave olhar para o Master
    float GetAngleToTarget(Vec3 slavePos, Vec3 masterPos) {
        float dx = masterPos.x - slavePos.x;
        float dy = masterPos.y - slavePos.y;
        return atan2f(dy, dx); // Retorna em radianos
    }

    // Decide quais teclas apertar
    void ProcessFollow(HWND hwnd, Vec3 slavePos, Vec3 masterPos) {
        float dx = masterPos.x - slavePos.x;
        float dy = masterPos.y - slavePos.y;
        float distance = sqrtf(dx * dx + dy * dy);

        // Margem de erro para evitar que o bot fique "tremendo" (Deadzone)
        const float margin = 3.0f;

        if (distance > 5.0f) { // Só se move se estiver a mais de 5 metros

            // --- Eixo Vertical (W / S) ---
            if (dy > margin)      PostMessage(hwnd, WM_KEYDOWN, VK_W, 0);
            else if (dy < -margin) PostMessage(hwnd, WM_KEYDOWN, VK_S, 0);
            else { PostMessage(hwnd, WM_KEYUP, VK_W, 0); PostMessage(hwnd, WM_KEYUP, VK_S, 0); }

            // --- Eixo Horizontal (A / D) ---
            if (dx > margin)      PostMessage(hwnd, WM_KEYDOWN, VK_D, 0);
            else if (dx < -margin) PostMessage(hwnd, WM_KEYDOWN, VK_A, 0);
            else { PostMessage(hwnd, WM_KEYUP, VK_A, 0); PostMessage(hwnd, WM_KEYUP, VK_D, 0); }

        }
        else {
            // Para totalmente se estiver perto o suficiente
            PostMessage(hwnd, WM_KEYUP, VK_W, 0);
            PostMessage(hwnd, WM_KEYUP, VK_S, 0);
            PostMessage(hwnd, WM_KEYUP, VK_A, 0);
            PostMessage(hwnd, WM_KEYUP, VK_D, 0);
        }
    }
};
