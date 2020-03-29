BUFF_LEN        equ 50
PERM_LEN        equ 42
SYS_READ          equ 0
SYS_WRITE       equ 1
SYS_EXIT        equ 60

STDIN           equ 0
STDOUT          equ 1
STDERR          equ 2

global _start

section .bss
buffer          resb BUFF_LEN+1
occur           resb PERM_LEN
permL           resb 8
permR           resb 8
permT           resb 8
key             resb 8
r               resb 1
l               resb 1

section .text

%macro checkParam 4
    mov         rdx, %3
    mov         bl,  %2
    mov         bh,  %4
    mov         rsi, %1
    call        _check_param
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

; input - kod znaku w al
_check_sign: 
    cmp         al, '1'+PERM_LEN-1      ; porownaj kod znaku z najwieszym mozliwym
    jg          _error                  ; jesli jest wiekszy niz 'Z' koncze program
    cmp         al, '1'                 ; porownuje znak z najmniejszym mozliwym
    jl          _error                  ; jesli jest mniejszy niz '1' koncze program
    sub         al, '1'                 ; wszystkie kody sprowadzam do przedzialu <0, 41>
    ret

;input : rsi string
_check_param:
    xor         rax, rax                ; potrzebna bedzie dokladna wartosc rejestru
    xor         rcx, rcx                ; zeruj licznik znakow
_check_param_loop:
    mov         al, [rsi]               ; aktualny znak
    test        al, al                  ; czy aktualny znak to '\0'?
    jz          _check_param_end        ; jesli tak zakoncz petle
    call        _check_sign             ; sprawdz poprawnosc znaku
    mov         [rsi], al               ; wstaw do tablicy zmniejszony kod znaku
    cmp         bl, PERM_LEN            ; czy sprawdzamy aktualnie permutacje?
    jne         _no_perm                ; jesli nie - nie sprawdzamy ilosci wystepowan znakow
    cmp         bl, [occur+rax]         ; porownaj iloc wystepowan znaku z oczekiwana iloscia
    jne         _error                  ; zakoncz program jesli znak wystepuje zla ilosc razy
_no_perm:
    mov         [occur+rax], dl         ; wstaw nowa ilosc wystepowan
    inc         rsi                     ; przesun wskaznik na kolejna litere
    inc         rcx                     ; zwiekszony licznik znakow
    jmp         _check_param_loop
_check_param_end:
    cmp         cl, bh                  ; porownaj z oczekiwana dlugoscia parametru
    jne         _error                  ; zakoncz jesli dlugosc jest inna
    ret

; input  kod znaku w al
_parse_sign:
    inc         bh                      ; zwieksz pozycje bębenka r
    cmp         bh, 'L'-'1'             ; czy r jest na L?
    je          _turn                   ; jesli tak przekrec l
    cmp         bh, 'R'-'1'             ; czy r jest na R?
    je          _turn                   ; jestli tak przekrec l
    cmp         bh, 'T'-'1'             ; czy r jest na T?
    je          _turn                   ; jesli tak przekrec l
    jmp         _no_turn                ; r nie jest na pozycji obrotowej
_turn:
    inc         bl                      ; zwieksz pozycje bebenka l
_no_turn:
    mod         bh                      ; moduluj przez mozliwa ilosc znakow kod bebenka r
    mod         bl                      ; moduluj przez mozliwa ilosc znakow kod bebenka l

    permQ       bh                      ; wykonaj perm Q_r
    perm        permR                   ; wykoanj perm R
    permQRev    bh                      ; wykonaj perm -Q_r
    permQ       bl                      ; wykonaj perm Q_l
    perm        permL                   ; wykonaj perm L
    permQRev    bl                      ; wykonaj perm -Q_l
    permRev     permT                   ; wykonaj perm T
    permQ       bl                      ; wykonaj perm Q_l
    permRev     permL                   ; wykonaj perm -L
    permQRev    bl                      ; wykonaj perm -Q_l
    permQ       bh                      ; wykonaj perm Q_r
    permRev     permR                   ; wykanaj perm -R
    permQRev    bh                      ; wykonaj perm -Q_r

    add         al, '1'
    ret
_start:
    pop          rax                    ; zdejmij ze stosu liczbe argumentow
    cmp          rax, 5                 ; ilość argumentów na wejsciu powinna wynoscic 5
    jne          _error                 ; zakoncz program z sygnalem bledu w przeciwnym przypadku

    pop         rax                     ; pierwszy argument to nazwa pliku wykonywalnego

    popTo       permL                   ; drugi argument to permutacja L
    popTo       permR                   ; trzeci to permutacja R
    popTo       permT                   ; czwarty to permutacja T
    popTo       key                     ; czwarty argument to klucz 

    checkParam  [permL], 0, 1, PERM_LEN ; sprawdz poprawnosc permutacji L
    checkParam  [permR], 1, 2, PERM_LEN ; sprawdz poprawnosc permutacji R
    checkParam  [permT], 2, 3, PERM_LEN ; sprawdz poprawnosc permutacji T
    checkParam  [key], 0, 0, 2          ; sprawdz poprawnosc klucza
_inputLoop:
    mov         rax, SYS_READ           ; ustaw do wywołania funkcję sys_read
    mov         rdi, STDIN              ; korzystaj ze standardowego wejscia
    mov         rdx, BUFF_LEN           ; wczytaj max BUFF_LEN znaków
    mov         rsi, buffer             ; wczytaj do bufora
    syscall                             ; wywołaj funkcję systemową
                      
    test        rax, rax                ; jesli wczytano 0 ilosc znakow
    jz          _inputEnd               ; skoncz wczytywanie

    push        rax                     ; ilosc wczytanych znakow przyda sie pozniej
    mov         rcx, rax                ; licznik petli - ilosc wczytanych znakow, idziemy po znakach od konca       
    mov         bx, [r]                 ; kopiujemy wartosci kluczy do rejestrow bh, bl
_checkLoop:
    mov         al, [buffer+rcx-1]      ; kopiuj aktualny znak    
    call        _check_sign             ; czy rozpatrywany znak jest poprawny?
    push        rcx
    call        _parse_sign             ; koduj znak
    pop         rcx
    mov         [buffer+rcx-1], al      ; wstaw zakodowany znak do bufora
    loop        _checkLoop              ; kontynuuj tylko jeśli są znaki do przejrzenia 

    mov         [r], bx                 ; wstaw spowrotem do pamieci klucze

    mov         rax, SYS_WRITE          ; ustaw do wywołania funkcję sys_write
    mov         rdi, STDOUT             ; korzystaj ze standarowego wyjścia
    pop         rdx                     ; zdejmij ze stosu ilość wpisanych wczesniej znaków
    mov         rsi, buffer             ; wypisuj z bufora
    syscall                             ; wywołaj funkcję systemową

    jmp         _inputLoop              ; czytaj kolejny blok
_inputEnd:
    exit        0