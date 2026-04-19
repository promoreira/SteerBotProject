#include "Injector.h"
#include <psapi.h>
#include <stdio.h> // Essencial para o vsnprintf

void DebugLog(const char* format, ...) {
    char buffer[512]; // Um "espaço" para montar a frase
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Agora sim chamamos a função oficial do Windows com a frase pronta
    OutputDebugStringA(buffer);
}

// Função para injetar a DLL (padrão LoadLibrary)
bool InjectDLL(HANDLE hProcess, const char* dllPath) {
    OutputDebugStringA("[SteerSvc] Tentando injetar DLL...");
    // Verifique se o caminho existe antes de tentar injetar
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
        OutputDebugStringA("[SteerSvc] ERRO: Arquivo DLL nao encontrado!");
        MessageBoxA(NULL, "ERRO: Arquivo DLL não encontrado na pasta Build!", "Injector", MB_OK);
        return false;
    }

    LPVOID remoteBuf = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteBuf) return false;

    WriteProcessMemory(hProcess, remoteBuf, (LPVOID)dllPath, strlen(dllPath) + 1, NULL);

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, remoteBuf, 0, NULL);
    if (!hThread) return false;

    // Se travar aqui, é porque o LoadLibrary no jogo travou
    WaitForSingleObject(hThread, 2000); // Espera no máximo 2 segundos
    CloseHandle(hThread);
    return true;
}


// O segredo para o Multi-Slave: Encontrar a variável exportada no Slave
uintptr_t GetRemoteExportAddress(DWORD pid, const char* moduleName, const char* exportName) {
    DebugLog("[SteerSvc] Buscando export %s no PID %d...", exportName, pid);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap == INVALID_HANDLE_VALUE) {
        DebugLog("[SteerSvc] ERRO: Falha ao criar snapshot do processo.");
        return 0;
    }

    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    uintptr_t remoteBase = 0;

    if (Module32First(hSnap, &me)) {
        do {
            if (_stricmp(me.szModule, moduleName) == 0) {
                remoteBase = (uintptr_t)me.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnap, &me));
    }
    CloseHandle(hSnap);

    if (!remoteBase) {
        DebugLog("[SteerSvc] ERRO: Modulo %s nao encontrado no Slave.", moduleName);
        return 0;
    }

    // Calcula o offset localmente
    HMODULE hLocal = LoadLibraryExA(moduleName, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!hLocal) {
        DebugLog("[SteerSvc] ERRO: Nao conseguiu carregar %s localmente.", moduleName);
        return 0;
    }
    uintptr_t localExport = (uintptr_t)GetProcAddress(hLocal, exportName);
    if (!localExport) {
        DebugLog("[SteerSvc] ERRO: Variavel %s nao encontrada na DLL.", exportName);
        FreeLibrary(hLocal);
        return 0;
    }
    uintptr_t offset = localExport - (uintptr_t)hLocal;
    FreeLibrary(hLocal);

    uintptr_t finalAddr = remoteBase + offset;
    DebugLog("[SteerSvc] SUCESSO: %s achado em 0x%p", exportName, (void*)finalAddr);

    return finalAddr;
}
