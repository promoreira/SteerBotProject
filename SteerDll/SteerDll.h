// O bloco ifdef a seguir é a forma padrão de criar macros que tornam a exportação
// de uma DLL mais simples. Todos os arquivos nessa DLL são compilados com STEERDLL_EXPORTS
// símbolo definido na linha de comando. Esse símbolo não deve ser definido em nenhum projeto
// que usa esta DLL. Desse modo, qualquer projeto cujos arquivos de origem incluem este arquivo veem
// Funções STEERDLL_API como importadas de uma DLL, enquanto esta DLL vê símbolos
// definidos com esta macro conforme são exportados.
#ifdef STEERDLL_EXPORTS
#define STEERDLL_API __declspec(dllexport)
#else
#define STEERDLL_API __declspec(dllimport)
#endif

// Esta classe é exportada da DLL
class STEERDLL_API CSteerDll {
public:
	CSteerDll(void);
	// TODO: adicione seus métodos aqui.
};

extern STEERDLL_API int nSteerDll;

STEERDLL_API int fnSteerDll(void);
