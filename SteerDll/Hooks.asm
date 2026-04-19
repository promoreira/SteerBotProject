.code
extern _playerbase: qword
extern fpReturnAddress: qword

PUBLIC hook_capture_pos
hook_capture_pos PROC
    ; 1. Captura o RAX (onde o player está sentado)
    mov qword ptr [_playerbase], rax
    
    ; 2. EXECUTA AS 3 INSTRUÇÕES ORIGINAIS (14 bytes totais)
    ; Essas instruções são o que fazem o boneco se mover no jogo
    movss xmm2, dword ptr [rax + 08h] ; Z
    movss xmm1, dword ptr [rax + 04h] ; Y
    movss xmm0, dword ptr [rax]       ; X
    
    ; 3. Volta para o jogo exatamente após os 14 bytes
    jmp qword ptr [fpReturnAddress]
hook_capture_pos ENDP
END
