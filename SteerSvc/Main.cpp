#include "Window.h"
#include "BotLogic.h"
#include "Injector.h"
#include "SteerPilot.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <vector>

// --- VARIÁVEIS GLOBAIS DE CONTROLE ---
bool g_FollowEnabled = false;
bool g_AntiAfkEnabled = false;

// Adicione apenas esta linha para o ImGui funcionar:
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// --- LÓGICA DE DESCOBERTA E INJEÇÃO ---
void DiscoverSlaves() {
    HWND hwnd = NULL;
    // Classe de janela padrão do DCUO (Unreal)
    while ((hwnd = FindWindowExA(NULL, hwnd, "LaunchUnrealUWindowsClient", NULL)) != NULL) {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);

        bool exists = false;
        for (const auto& s : g_activeSlaves) if (s.pid == pid) exists = true;
        if (exists) continue;

        SlaveInstance newSlave = { 0 };
        newSlave.pid = pid;
        newSlave.hwnd = hwnd;
        newSlave.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        newSlave.isConnected = false;

        char dllPath[MAX_PATH];
        GetFullPathNameA("SteerDll.dll", MAX_PATH, dllPath, NULL);

        if (InjectDLL(newSlave.hProcess, dllPath)) {
            Sleep(800); // Tempo para o Hook estabilizar
            newSlave.addrRemoteState = GetRemoteExportAddress(pid, "SteerDll.dll", "g_SlaveState");

            if (newSlave.addrRemoteState != 0) {
                newSlave.isConnected = true;
                g_activeSlaves.push_back(newSlave);
            }
        }
    }
}

// --- ENTRY POINT PRINCIPAL ---
int main(int, char**) {
    // 1. Criar Janela Win32
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, App::WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "SteerBotClass", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, "SteerBot Pro v3.5 - Professor IA", WS_OVERLAPPEDWINDOW, 100, 100, 1000, 600, NULL, NULL, wc.hInstance, NULL);

    // 2. Inicializar DirectX 11
    if (!App::CreateDeviceD3D(hwnd)) {
        App::CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // 3. Inicializar Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(App::g_pd3dDevice, App::g_pd3dDeviceContext);

    // 4. Loop de Mensagens e Lógica do Bot
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // --- ATUALIZAÇÃO DOS DADOS ---
        UpdateSlaves(g_activeSlaves);

        // --- LÓGICA DE MOVIMENTO (SOMENTE NOS SLAVES) ---
        if (g_FollowEnabled && g_activeSlaves.size() >= 2) {
            // O primeiro (índice 0) é o mestre
            Vec3 masterPos = g_activeSlaves[0].localData.currentPos;

            for (size_t i = 1; i < g_activeSlaves.size(); i++) {
                if (g_activeSlaves[i].isConnected) {
                    g_activeSlaves[i].pilot.ProcessFollow(g_activeSlaves[i].hwnd, g_activeSlaves[i].localData.currentPos, masterPos);
                }
            }
        }
        else if (!g_FollowEnabled) {
            // Garante que os slaves parem se o follow for desligado
            for (size_t i = 1; i < g_activeSlaves.size(); i++) {
                g_activeSlaves[i].pilot.StopAll(g_activeSlaves[i].hwnd);
            }
        }

        // --- RENDERIZAÇÃO DA INTERFACE ---
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Painel de Controle SteerBot v3.5");

            if (ImGui::Button("Escanear Janelas", ImVec2(150, 30))) {
                DiscoverSlaves();
            }

            ImGui::SameLine();
            ImGui::Checkbox("Ativar Follow (Yaw Estimado)", &g_FollowEnabled);

            if (ImGui::Checkbox("Anti-AFK (DCGAME.EXE Patch)", &g_AntiAfkEnabled)) {
                for (auto& s : g_activeSlaves) {
                    uintptr_t base = GetModuleBase(s.pid, "DCGAME.EXE");
                    PatchAntiAfk(s.hProcess, base, g_AntiAfkEnabled);
                }
            }

            ImGui::Separator();
            ImGui::Text("Status dos Personagens:");
            for (int i = 0; i < (int)g_activeSlaves.size(); i++) {
                auto& s = g_activeSlaves[i];
                ImVec4 col = s.isConnected ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);

                ImGui::TextColored(col, "[%s] PID: %d | X: %.1f Y: %.1f Z: %.1f",
                    (i == 0 ? "MESTRE" : "SLAVE"), s.pid, s.localData.currentPos.x, s.localData.currentPos.y, s.localData.currentPos.z);
            }

            if (ImGui::Button("Fechar Bot", ImVec2(100, 25))) PostQuitMessage(0);
            ImGui::End();
        }

        // Renderização Final
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.10f, 0.10f, 0.10f, 1.00f };
        App::g_pd3dDeviceContext->OMSetRenderTargets(1, &App::g_mainRenderTargetView, NULL);
        App::g_pd3dDeviceContext->ClearRenderTargetView(App::g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        App::g_pSwapChain->Present(1, 0);
        Sleep(10); // Alívio para a CPU
    }

    // 5. Limpeza Final
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    App::CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
