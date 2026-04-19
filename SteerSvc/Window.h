#pragma once
#include <windows.h>
#include <d3d11.h>

namespace App {
    // Inicializa a janela e o DirectX 11
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    // Função que processa as mensagens do Windows (fechar, redimensionar, etc)
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Variáveis globais para o DirectX (acessíveis pelo Window.cpp)
    extern ID3D11Device* g_pd3dDevice;
    extern ID3D11DeviceContext* g_pd3dDeviceContext;
    extern IDXGISwapChain* g_pSwapChain;
    extern ID3D11RenderTargetView* g_mainRenderTargetView;
}
