BUFF_LEN    equ 5
PERM_LEN    equ 42
SYS_IN      equ 0
SYS_WRITE   equ 1
SYS_EXIT    equ 60

STDIN       equ 0
STDOUT      equ 1
STDERR      equ 2

global _start

section .bss
buffer      resb BUFF_LEN+1
occur       resb PERM_LEN
permL       resb 8
permR       resb 8
permT       resb 8
key         resb 8
r           resb 1
l           resb 1

section .text

%macro checkParam 3
    mov         rdx, %3
    mov         rbx, %2
    mov         rsi, %1
    call        _check_param
%endmacro

%macro checkKey 1
    mov         rsi, %1
    mov         al, [rsi]
    call        _check_sign
    mov         [r], al
    mov         al, [rsi+1]
    call        _check_sign
    mov         [l], al
    mov         al, [rsi+2]
    test        al, al
    jnz         _error
%endmacro

%macro  popTo    1
    pop         rax
    mov         [%1], rax 
%endmacro

%macro checkSign 1
    mov         al, %1
    call        _check_sign
%endmacro

%macro mod 1
    cmp         %1, PERM_LEN
    jl          %%no_mod
    sub         %1, PERM_LEN
%%no_mod:
%endmacro

%macro  exit 1
    mov     rax, SYS_EXIT
    mov     rdi, %1
    syscall
%endmacro

%macro permQ    1
    add         al, %1 
    mod         al
%endmacro

%macro permQRev 1
    add         al, PERM_LEN
    sub         al, %1 
    mod         al
%endmacro

%macro permRev 1
    mov         rcx, PERM_LEN-1
    mov         rdx, [%1]
%%find_r:
    cmp         al, [rdx+rcx]
    je          %%find_r_end
    loop        %%find_r
%%find_r_end:
    mov         rax, rcx
%endmacro

%macro perm 1
    mov         rdx, [%1]
    mov         al, [rdx+rax]
%endmacro

_error:
    exit        1

_input:
    mov         rax, SYS_IN
    mov         rdi, STDIN
    mov         rdx, BUFF_LEN
    mov         rsi, buffer
    syscall

    ret

; input - kod znaku w al
_check_sign: 
    cmp         al, '1'+PERM_LEN-1  ; porownuje kod znaku z najwieszym mozliwym
    jg          _error              ; jesli jest wiekszy niz 'Z' koncze program
    cmp         al, '1'             ; porownuje znak z najmniejszym mozliwym
    jl          _error              ; jesli jest mniejszy niz '1' koncze program
    sub         al, '1'             ; wszystkie kody sprowadzam do przedzialu <0, 41>
    ret

;input : rsi string
_check_param:
    xor         rax, rax            ; bede potrzebowal dokladnej wartosci raxa
    xor         rcx, rcx            ; zeruje licznik znakow
_check_param_loop:
    mov         al, [rsi]           ; aktualny znak
    test        al, al              ; 0 konczy tekst
    jz          _check_param_end    ; koncze sprawdzanie jesli 0
    call        _check_sign         ; sprawdzam poprawnosc znaku
    mov         [rsi], al           ; w al jest teraz zmniejszony o '1' kod
    cmp         bl, [occur+rax]     ; porownaj iloc wystepowan znaku z oczekiwana iloscia
    jne         _error              ; zakoncz program jesli znak wystepuje zla ilosc razy
    mov         [occur+rax], dl     ; wstaw nowa ilosc wystepowan
    inc         rsi                 ; przesuwam wskaznik na kolejna litere
    inc         rcx                 ; zwiekszony licznik znakow
    jmp         _check_param_loop
_check_param_end:
    cmp         rcx, PERM_LEN       ; paramtr powinien byc dlugosci PERM_LEN
    jne         _error              ; jesli nie jest to koncze program
    ret

; input  kod znaku w al
_parse_sign:
    inc         bh
    cmp         bh, 'L'-'1'
    je          _turn
    cmp         bh, 'R'-'1'
    je          _turn
    cmp         bh, 'T'-'1'
    je          _turn
    jmp         _no_turn
_turn:
    inc         bl
_no_turn:
    mod         bh
    mod         bl

    permQ       bh
    perm        permR
    permQRev    bh
    permQ       bl
    perm        permL
    permQRev    bl
    permRev     permT
    permQ       bl
    permRev     permL
    permQRev    bl
    permQ       bh
    permRev     permR
    permQRev    bh       

    add         al, '1'
    ret
_start:
    pop          rax                ; zdejmujemy liczbe argumentow
    cmp          rax, 5             ; ilość argumentów na wejsciu powinna wynoscic 5
    jne          _error             ; konczymy program z sygnalem bledu w przeciwnym przypadku

    pop         rax                 ; pierwszy argument to nazwa pliku wykonywalnego
    popTo       permL               ; drugi argument to permutacja L
    popTo       permR               ; trzeci to permutacja R
    popTo       permT               ; czwarty to permutacja T
    popTo       key                 ; czwarty argument to klucz 

    checkParam  [permL], 0, 1       ; sprawdzamy poprawnosc permutacji L
    checkParam  [permR], 1, 2       ; sprawdzamy poprawnosc permutacji R
    checkParam  [permT], 2, 3       ; sprawdzamy poprawnosc permutacji T

    checkKey    [key]               ; sprawdamy poprawnosc klucza
_inputLoop:
    call        _input              ; czytaj blok do bufora
    test        rax, rax            ; jesli wczytalismy 0 ilosc znakow
    jz          _inputEnd           ; skoncz wczytywanie

    push        rax                 ; ilosc wczytanych znakow przyda sie pozniej
    mov         rcx, rax            ; licznik petli - ilosc wczytanych znakow, idziemy po znakach od konca       
    mov         bx, [r]             ; kopiujemy wartosci kluczy do rejestrow bh, bl
_checkLoop:
    mov         al, [buffer+rcx-1]  ; kopiujemy aktualny znak    
    call        _check_sign         ; czy rozpatrywany znak jest poprawny?
    push        rcx
    call        _parse_sign         ; kodujemy znak
    pop         rcx
    mov         [buffer+rcx-1], al  ; wstawiamy zakodowany znak do bufora
    loop        _checkLoop          ; kontynuuj tylko jeśli są znaki do przejrzenia 

    mov         [r], bx             ; wstawiamy spowrotem do pamieci klucze

    mov         rax, SYS_WRITE      ; bedziemy wypisywac
    mov         rdi, STDOUT         ; na standadowe wyjscie
    pop         rdx                 ; zdejmujemy ilosc wczytanych znakow
    mov         rsi, buffer         ; wypisujemy dane z bufora
    syscall

    jmp         _inputLoop          ; czytaj kolejny blok
_inputEnd:
    exit        0