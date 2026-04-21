#pragma once
#include <windows.h>
#include <cmath>
#include "SharedData.h"

class SteerPilot {
private:
    Vec3 lastPos = { 0, 0, 0 };
    float estimatedYaw = 0.0f;
    const float PI = 3.1415926535f;

public:
    void ProcessFollow(HWND hwnd, Vec3 slavePos, Vec3 masterPos) {
        // 1. Estima a direção baseada no movimento real (Aprendizado)
        float moveX = slavePos.x - lastPos.x;
        float moveY = slavePos.y - lastPos.y;
        float moveDist = sqrtf(moveX * moveX + moveY * moveY);

        if (moveDist > 0.05f) {
            estimatedYaw = atan2f(moveY, moveX) * (180.0f / PI);
        }
        lastPos = slavePos;

        // 2. Calcula o ângulo para o Mestre
        float dx = masterPos.x - slavePos.x;
        float dy = masterPos.y - slavePos.y;
        float targetAngle = atan2f(dy, dx) * (180.0f / PI);
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance > 5.0f) {
            float angleDiff = targetAngle - estimatedYaw;

            // Normaliza o ângulo (-180 a 180)
            while (angleDiff > 180) angleDiff -= 360;
            while (angleDiff < -180) angleDiff += 360;

            // --- 3. ROTAÇÃO VIA TECLADO (A / D) ---
            // Em vez de mexer o mouse, fazemos o boneco girar pressionando A ou D
            if (angleDiff > 15.0f) {
                PostMessage(hwnd, WM_KEYDOWN, 'D', 0);
                PostMessage(hwnd, WM_KEYUP, 'A', 0);
            }
            else if (angleDiff < -15.0f) {
                PostMessage(hwnd, WM_KEYDOWN, 'A', 0);
                PostMessage(hwnd, WM_KEYUP, 'D', 0);
            }
            else {
                // Se estiver alinhado, solta as teclas de giro
                PostMessage(hwnd, WM_KEYUP, 'A', 0);
                PostMessage(hwnd, WM_KEYUP, 'D', 0);
            }

            // --- 4. MOVIMENTO FRENTE (W) ---
            // Só aperta W se estiver minimamente apontado para o alvo (margem de 45 graus)
            if (fabs(angleDiff) < 45.0f) {
                PostMessage(hwnd, WM_KEYDOWN, 'W', 0);
            }
            else {
                PostMessage(hwnd, WM_KEYUP, 'W', 0);
            }
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
