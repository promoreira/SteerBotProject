#pragma once
#include <windows.h>
#include <tlhelp32.h>

// Função que coloca a DLL dentro do processo do Slave
bool InjectDLL(HANDLE hProcess, const char* dllPath);

// Função que localiza o endereço da variável 'g_SlaveState' dentro do Slave
// Isso resolve o problema do ASLR (endereços diferentes em cada janela)
uintptr_t GetRemoteExportAddress(DWORD pid, const char* moduleName, const char* exportName);
