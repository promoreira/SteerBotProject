#pragma once
#include <windows.h>
#include <cmath>
#include "SharedData.h"

class SteerPilot {
private:
    Vec3 lastPos = { 0, 0, 0 };
    float estimatedYaw = 0.0f;

public:
    void ProcessFollow(HWND hwnd, Vec3 slavePos, Vec3 masterPos) {
        // 1. CALCULAR O YAW ESTIMADO (Para onde o boneco está realmente indo)
        float moveX = slavePos.x - lastPos.x;
        float moveZ = slavePos.z - lastPos.z;
        float moveDist = sqrtf(moveX * moveX + moveZ * moveZ);

        // Só atualiza o Yaw se o boneco estiver se movendo (evita ruído parado)
        if (moveDist > 0.1f) {
            estimatedYaw = atan2f(moveZ, moveX);
        }
        lastPos = slavePos; // Guarda para o próximo frame

        // 2. CALCULAR DIREÇÃO PARA O MESTRE
        float dx = masterPos.x - slavePos.x;
        float dz = masterPos.z - slavePos.z;
        float distance = sqrtf(dx * dx + dz * dz);

        if (distance > 30.0f) {
            // Ângulo global para o mestre
            float angleToMaster = atan2f(dz, dx);

            // Diferença entre para onde estou indo e para onde devo ir
            float relativeAngle = angleToMaster - estimatedYaw;

            // 3. TRADUZIR PARA COMANDOS WASD
            // Local Frente (Z) e Lado (X) relativo ao boneco
            float localZ = cosf(relativeAngle);
            float localX = sinf(relativeAngle);

            // Frente / Trás
            if (localZ > 0.3f) { PostMessage(hwnd, WM_KEYDOWN, 'W', 0); PostMessage(hwnd, WM_KEYUP, 'S', 0); }
            else if (localZ < -0.3f) { PostMessage(hwnd, WM_KEYDOWN, 'S', 0); PostMessage(hwnd, WM_KEYUP, 'W', 0); }
            else { PostMessage(hwnd, WM_KEYUP, 'W', 0); PostMessage(hwnd, WM_KEYUP, 'S', 0); }

            // Esquerda / Direita
            if (localX > 0.3f) { PostMessage(hwnd, WM_KEYDOWN, 'D', 0); PostMessage(hwnd, WM_KEYUP, 'A', 0); }
            else if (localX < -0.3f) { PostMessage(hwnd, WM_KEYDOWN, 'A', 0); PostMessage(hwnd, WM_KEYUP, 'D', 0); }
            else { PostMessage(hwnd, WM_KEYUP, 'A', 0); PostMessage(hwnd, WM_KEYUP, 'D', 0); }
        }
        else {
            StopAll(hwnd);
        }
    }

    void StopAll(HWND hwnd) {
        PostMessage(hwnd, WM_KEYUP, 'W', 0);
        PostMessage(hwnd, WM_KEYUP, 'S', 0);
        PostMessage(hwnd, WM_KEYUP, 'A', 0);
        PostMessage(hwnd, WM_KEYUP, 'D', 0);
    }
};
