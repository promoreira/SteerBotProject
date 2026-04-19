#include "Window.h"
#include "BotLogic.h"
#include "Injector.h"
#include "SteerPilot.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <vector>

// Variáveis Globais de Controle
bool g_Running = true;
bool g_FollowEnabled = false;
static bool g_AntiAfkEnabled = false; // Controle do Checkbox

// Variáveis Estáticas para o Patch Anti-AFK
static BYTE s_originalAntiAfk[8] = { 0 };
static bool s_antiAfkBytesSaved = false;

// Função para buscar as janelas e injetar a DLL
void DiscoverSlaves() {
    HWND hwnd = NULL;
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
            Sleep(800);
            newSlave.addrRemoteState = GetRemoteExportAddress(pid, "SteerDll.dll", "g_SlaveState");

            if (newSlave.addrRemoteState != 0) {
                newSlave.isConnected = true;
                g_activeSlaves.push_back(newSlave);
            }
        }
    }
}

int main() {
    // 1. Configuração da Janela (Win32)
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, App::WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "SteerBotClass", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, "SteerBot - Professor de IA", WS_OVERLAPPEDWINDOW, 100, 100, 1000, 600, NULL, NULL, wc.hInstance, NULL);

    // 2. Inicialização do DirectX 11
    if (!App::CreateDeviceD3D(hwnd)) {
        App::CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // 3. Inicialização do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(App::g_pd3dDevice, App::g_pd3dDeviceContext);

    // 4. Loop Principal
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        UpdateSlaves(g_activeSlaves);

        // --- LÓGICA DE MOVIMENTO ---
        if (g_FollowEnabled && g_activeSlaves.size() >= 2) {
            SlaveInstance& master = g_activeSlaves[0];
            for (size_t i = 1; i < g_activeSlaves.size(); i++) {
                SlaveInstance& slave = g_activeSlaves[i];
                if (master.isConnected && slave.isConnected) {
                    slave.pilot.ProcessFollow(slave.hwnd, slave.localData.currentPos, master.localData.currentPos);
                }
            }
        }
        else if (!g_FollowEnabled) {
            for (auto& s : g_activeSlaves) {
                s.pilot.StopAll(s.hwnd);
            }
        }

        // --- RENDERIZAÇÃO INTERFACE ---
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Painel de Controle SteerBot");

            if (ImGui::Button("Escanear Slaves", ImVec2(150, 30))) {
                DiscoverSlaves();
            }

            ImGui::SameLine();
            ImGui::Checkbox("Ativar Follow (Yaw Estimado)", &g_FollowEnabled);

            // --- SEÇÃO ANTI-AFK ---
            if (ImGui::Checkbox("Ativar Anti-AFK (Patch)", &g_AntiAfkEnabled)) {
                for (auto& s : g_activeSlaves) {
                    if (s.isConnected) {
                        // Tenta obter a base do executável do jogo
                        uintptr_t base = GetModuleBase(s.pid, "DCGAME.EXE");
                        PatchAntiAfk(s.hProcess, base, g_AntiAfkEnabled);
                    }
                }
            }

            ImGui::Separator();
            ImGui::Text("Slaves Ativos: %d", (int)g_activeSlaves.size());

            for (int i = 0; i < (int)g_activeSlaves.size(); i++) {
                auto& s = g_activeSlaves[i];
                ImVec4 color = s.isConnected ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::Text("[%d] PID: %d | X: %.1f Y: %.1f Z: %.1f", i, s.pid, s.localData.currentPos.x, s.localData.currentPos.y, s.localData.currentPos.z);
                if (i == 0) { ImGui::SameLine(); ImGui::Text(" (MESTRE)"); }
                ImGui::PopStyleColor();
            }

            if (ImGui::Button("Fechar Bot", ImVec2(100, 20))) msg.message = WM_QUIT;
            ImGui::End();
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.10f, 0.10f, 0.10f, 1.00f };
        App::g_pd3dDeviceContext->OMSetRenderTargets(1, &App::g_mainRenderTargetView, NULL);
        App::g_pd3dDeviceContext->ClearRenderTargetView(App::g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        App::g_pSwapChain->Present(1, 0);
        Sleep(10);
    }

    // 5. Limpeza
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    App::CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
