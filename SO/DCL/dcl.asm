BUFF_LEN        equ 4096
PERM_LEN        equ 42
SYS_READ        equ 0
SYS_WRITE       equ 1
SYS_EXIT        equ 60

STDIN           equ 0
STDOUT          equ 1
STDERR          equ 2

global _start

section .bss
buffer          resb BUFF_LEN+1
occur           resb PERM_LEN
permRevL        resb PERM_LEN
permRevR        resb PERM_LEN
permL           resb 8
permR           resb 8
permT           resb 8       
key             resb 8
l               resb 1
r               resb 1

section .text

%macro checkSign 1
    mov         al, %1
    call        _check_sign
%endmacro

%macro checkParam 4
    mov         rdx, %3
    mov         bl,  %2
    mov         bh,  %4
    mov         rsi, %1
    call        _check_param
%endmacro

%macro popTo 1
    pop         rax
    mov         [%1], rax 
%endmacro

%macro  exit 1
    mov         rax, SYS_EXIT
    mov         rdi, %1
    syscall
%endmacro

%macro mod 1
    cmp         %1, PERM_LEN
    jl          %%no_mod
    sub         %1, PERM_LEN
%%no_mod:
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

; Wywoluje procedure _create_rev obliczającą permutację odwrotną
; argumenty:    1 - adres permutacji
;               2 - adres wynikowej permutacji odwrotnej
%macro createRev 2
    mov         rax, %1
    mov         rbx, %2
    call        _create_rev
%endmacro

; Makro zamieniające znak korzystajac z odpowiedniej permutacji
; argumenty:    1: adres permutacji
; wejscie:      kod znaku w rax
; wyjcie:       przepermutowany kod znaku w rax
%macro perm 1
    mov         rdx, %1                 ; kopiuj do rdx adres permutacji
    mov         al, [rdx+rax]           ; wykonaj odpowiednia permutacje znaku
%endmacro


; Konczy program z sygnalem bledu
_error:
    exit        1

; Procedura obliczająca permutację odwrotną do danej
; wejscie:      rax - adres tablicy z permutacja
;               rbx - adres tablicy wynikowej permutacji 
; wyjscie:      wynikowa permutacja w tablicy pod rbx
_create_rev:
    xor         rdx, rdx                ; wartosc rdx bedzie rowna kodowi znaku
    mov         rcx, PERM_LEN-1         ; ustaw licznik petli do chodzenia po permutacji
_create_rev_loop:
    mov         dl, [rax+rcx]           ; wstaw do dl kod znaku z permutacji
    mov         [rbx+rdx], cl           ; wstaw odpowiedni znak do permutacji odwrotnej
    loop        _create_rev_loop        ; kontynuuj petle jesli licznik jest wiekszy od 0
    ret

; Sprawdza poprawnosc znaku i zmniejsza jego kod o '1'. Konczy program jesli znak jest niepoprawny
; wejscie:      al - kod znaku
; wyjscie       al - zmniejszony o '1' kod znaku 
_check_sign: 
    cmp         al, '1'+PERM_LEN-1      ; porownaj kod znaku z najwieszym mozliwym
    jg          _error                  ; jesli jest wiekszy niz 'Z' koncze program
    cmp         al, '1'                 ; porownuje znak z najmniejszym mozliwym
    jl          _error                  ; jesli jest mniejszy niz '1' koncze program
    sub         al, '1'                 ; wszystkie kody sprowadzam do przedzialu <0, 41>
    ret

; Procedura sprawdzająca poprawność permutacji. Konczy program z sygnalem bledu
; jesli rozpatrywana permutacja jest niepoprawna
; wejscie:      rsi - adres tablicy z permutacja
_check_param:
    xor         rax, rax                ; wartosc calego rax bedzie rowna kodowi znaku
    xor         rcx, rcx                ; zeruj licznik znakow
_check_param_loop:
    mov         al, [rsi]               ; wstaw do al kod znaku
    test        al, al                  ; czy aktualny znak to '\0'?
    jz          _check_param_end        ; jesli tak zakoncz petle
    call        _check_sign             ; sprawdz poprawnosc znaku
    mov         [rsi], al               ; wstaw spowrotem do bss zmniejszony kod znaku
    cmp         bl, PERM_LEN            ; czy sprawdzamy aktualnie permutacje?
    jne         _no_perm                ; jesli nie - nie sprawdzamy ilosci wystepowan znakow
    cmp         bl, [occur+rax]         ; porownaj iloc wystepowan znaku z oczekiwana iloscia
    jne         _error                  ; zakoncz program jesli znak wystepuje zla ilosc razy
_no_perm:
    mov         [occur+rax], dl         ; wstaw do bss nowa ilosc wystepowan znaku - dana w dl
    inc         rsi                     ; przesun wskaznik na kolejny znak 
    inc         rcx                     ; zwieksz licznik znakow
    jmp         _check_param_loop       ; kontynuuj sprawdzanie parametru
_check_param_end:
    cmp         cl, bh                  ; porownaj z oczekiwana dlugoscia parametru
    jne         _error                  ; zakoncz jesli dlugosc jest inna
    ret

; Procedura sprawdzajaca poprawnosc permutacji T
; konczy program z sygnalem bledu jesli permutacja jest niepoprawna
_check_t:
    mov         rcx, PERM_LEN-1         ; ustaw licznik petli do chodzenia po permutacji
    xor         rbx, rbx                ; wartosc rbx bedzie rowna kodowi znaku
    mov         rax, [permT]            ; kopiuj adres permutacji
_check_t_loop:
    mov         bl, [rax+rcx]           ; kopiuj do bl kod znaku
    cmp         bl, cl                  ; porownaj z pozycja w permutacji
    je          _error                  ; punkt staly permutacji T? blad 
    mov         dl, [rax+rbx]           ; znak odpowiadajacy atualnemu w dwuelementowym cyklu
    cmp         cl, dl                  ; czy ten znak jest rowny aktulnej pozycji?
    jne         _error                  ; nie? znaleziono cykl o dlugosci innej niz 2 - blad
    loop        _check_t_loop            ; kontunuuj petle jesli licznik jest wiekszy od 0
    
    ret


; Procedura zamieniająca znak korzystając ze złożenia permutacji
; wejscie:      kod znaku w rax
; wyjscie       przepermutowany kod znaku w rax 
_parse_sign:
    inc         bl                      ; zwieksz pozycje bębenka r
    cmp         bl, 'L'-'1'             ; czy r jest na L?
    je          _turn                   ; jesli tak przekrec l
    cmp         bl, 'R'-'1'             ; czy r jest na R?
    je          _turn                   ; jesli tak przekrec l
    cmp         bl, 'T'-'1'             ; czy r jest na T?
    je          _turn                   ; jesli tak przekrec l
    jmp         _no_turn                ; r nie jest na pozycji obrotowej
_turn:
    inc         bh                      ; zwieksz pozycje bebenka l
_no_turn:
    mod         bl                      ; moduluj przez mozliwa ilosc znakow kod bebenka r
    mod         bh                      ; moduluj przez mozliwa ilosc znakow kod bebenka l

    permQ       bl                      ; wykonaj perm Q_r
    perm        [permR]                 ; wykonaj perm R
    permQRev    bl                      ; wykonaj perm -Q_r
    permQ       bh                      ; wykonaj perm Q_l
    perm        [permL]                 ; wykonaj perm L
    permQRev    bh                      ; wykonaj perm -Q_l
    perm        [permT]                 ; wykonaj perm T
    permQ       bh                      ; wykonaj perm Q_l
    perm        permRevL                ; wykonaj perm -L
    permQRev    bh                      ; wykonaj perm -Q_l
    permQ       bl                      ; wykonaj perm Q_r
    perm        permRevR                ; wykanaj perm -R
    permQRev    bl                      ; wykonaj perm -Q_r

    add         al, '1'                 ; zmien spowrotem na znak do wypisania
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
    
    call        _check_t

    createRev   [permL], permRevL       ; tworz odwrotnosc perm L
    createRev   [permR], permRevR       ; tworz odwrotnosc perm R

    mov         rax, [key]              ; kopiuj do bss poczatkowe
    mov         bx, [rax]               ; wartosci bebenkow
    mov         [l], bx                 ; l i r (są obok siebie w pamięci)
_input_loop:
    mov         rax, SYS_READ           ; ustaw do wywołania funkcję sys_read
    mov         rdi, STDIN              ; korzystaj ze standardowego wejscia
    mov         rdx, BUFF_LEN           ; wczytaj max BUFF_LEN znaków
    mov         rsi, buffer             ; wczytaj do bufora
    syscall                             ; wywołaj funkcję systemową
                      
    test        rax, rax                ; jesli wczytano 0 ilosc znakow
    jz          _input_end              ; skoncz wczytywanie

    push        rax                     ; ilosc wczytanych znakow przyda sie pozniej
    xor         rcx, rcx                ; inicjalizuj licznik petli
    mov         bx, [l]                 ; kopiujemy wartosci kluczy do rejestrow bh, bl
_check_loop:
    mov         al, [buffer+rcx]        ; kopiuj aktualny znak z bufora
    call        _check_sign             ; czy rozpatrywany znak jest poprawny? 
    push        rcx                     ; _parse_sign korzysta z rcx - wrzuc go na stos
    call        _parse_sign             ; koduj znak
    pop         rcx                     ; zabierz spowrotem rcx
    mov         [buffer+rcx], al        ; wstaw zakodowany znak do bufora
    inc         rcx                     ; zwieksz licznik petli
    cmp         rcx, [rsp]              ; czy zostaly przejrzane wszystkie znaki? na gorze stosu jest liczba wczytanych znakow
    jne          _check_loop            ; jesli nie kontynuuj parsowanie bufora
    mov         [l], bx                 ; wstaw spowrotem do pamieci klucze

    mov         rax, SYS_WRITE          ; ustaw do wywołania funkcję sys_write
    mov         rdi, STDOUT             ; korzystaj ze standarowego wyjścia
    pop         rdx                     ; ustaw ilosc znakow do wypisania na ilosc wczytanych wczesniej znakow
    mov         rsi, buffer             ; wypisuj z bufora
    syscall                             ; wywołaj funkcję systemową

    jmp         _input_loop             ; czytaj kolejny blok
_input_end:
    exit        0