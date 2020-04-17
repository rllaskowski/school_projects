SYS_READ        equ 0
SYS_WRITE       equ 1
SYS_EXIT        equ 60

STDIN           equ 0
STDOUT          equ 1
STDERR          equ 2

BUFF_LEN        equ 4095
PERM_LEN        equ 42

global _start

section .bss
buffer          resb BUFF_LEN+1
occur           resb PERM_LEN+6
permRevL        resb PERM_LEN+6
permRevR        resb PERM_LEN+6
permL           resb 8
permR           resb 8
permT           resb 8

section .text

; Makro wywołujące procedurę _check_param
; argumenty:    %1 - adres w pamieci wskazujacy na parametr
;               %2 - oczekiwana wartosc occur[x] dla znaku z kodem x przed wywolaniem procedury 
;               %3 - oczekiwana długosc parametru
%macro checkParam 3
    mov         rsi, %1
    mov         bl,  %2
    mov         bh,  %3
    call        _check_param
%endmacro

; Makro zdejmujace wartosc ze stosu i kopiujace te wartosc pod dany adres
; argumenty:    %1 - adres w pamieci do skopiowania wartosci ze stosu
; wyjscie:      rax - zdjeta ze stosu wartosc
%macro popTo 1
    pop         rax                     
    mov         %1, rax 
%endmacro


; Makro wywolujace funkcję systemową sys_exit z odpowiednim sygnalem wyjscia
; argumenty:    %1 - sygnal wyjscia
%macro  exit 1
    mov         eax, SYS_EXIT           ; ustaw odpowiednia funkcje systemowa -sys_exit
    mov         edi, %1                 ; ustaw sygnal wyjscia
    syscall                         
%endmacro

; Makro modulujące liczbę nieujemną przez PERM_LEN
; argumenty:    %1 - rejestr z liczbą do modulowania
; wyjscie:      %1 - rejestr z przemodulowana liczba
%macro mod 1
    cmp         %1, PERM_LEN            ; maksymalna dopuszczalna wartosc to PERM_LEN-1
    jl          %%no_mod                ; jesli %1 jest w zakresie <0,PERM_LEN-1> nic nie rob
    sub         %1, PERM_LEN            ; odejmij PERM_LEN jesli %1 jest za duze
%%no_mod:
%endmacro

; Makro wykonujace permutacje Q
; argumenty:    %1 - argument permutacji Q - wartosc w <0,PERM_LEN-1>
; wejcie:       al - kod znaku w przediale <0,PERM_LEN-1>
; wyjsci:       al - przepermutowany kod znaku w przedziale <0,PERM_LEN-1>
%macro permQ    1
    add         al, %1 
    mod         al
%endmacro

; Makro wykonujace permutacje Q^-1
; argumenty:    %1 - argument permutacji Q^-1 - wartosc w <0,PERM_LEN-1>
; wejcie:       al - kod znaku w przediale <0,PERM_LEN-1>
; wyjscie:       al - przepermutowany kod znaku w przedziale <0,PERM_LEN-1>
%macro permQRev 1          
    add         al, PERM_LEN            ; dodanie PERM_LEN zapewnia, że al bedzie liczba calkowita
    sub         al, %1                  ; wykonaj permutacje Q^-1 z odpowiednim argumentem
    mod         al          
%endmacro

; Makro wwolujejace procedure _create_rev obliczającą permutację odwrotną
; argumenty:    %1 - adres permutacji
;               %2 - adres wynikowej permutacji odwrotnej
%macro createRev 2
    mov         rax, %1                 ; _create_rev przyjmuje w rax adres tablicy permutacji 
    mov         rbx, %2                 ; _create_rev przyjmuje w rbx adres wynikowej permutacji odwrotnej
    call        _create_rev
%endmacro

; Makro zamieniające znak korzystajac z odpowiedniej permutacji
; argumenty:    1 - adres permutacji
; wejscie:      kod znaku w rax
; wyjcie:       przepermutowany kod znaku w al
%macro perm 1
    mov         rsi, %1                 ; kopiuj do rsi adres permutacji
    mov         al, [rsi+rax]           ; wykonaj odpowiednia permutacje znaku
%endmacro


; Konczy program z sygnalem bledu
_error:
    exit        1

; Procedura obliczająca permutację odwrotną do danej
; wejscie:      rax - adres tablicy z permutacja
;               rbx - adres tablicy wynikowej permutacji 
; wyjscie:      wynikowa permutacja w tablicy pod rbx
_create_rev:
    xor         edx, edx                ; wartosc rdx bedzie rowna kodowi znaku
    mov         rcx, PERM_LEN-1         ; ustaw licznik petli do chodzenia po permutacji
_create_rev_loop:
    mov         dl, [rax+rcx]           ; wstaw do dl kod znaku z permutacji
    mov         [rbx+rdx], cl           ; wstaw odpowiedni znak do permutacji odwrotnej
    loop        _create_rev_loop        ; kontynuuj petle jesli licznik jest wiekszy od 0
    ret

; Sprawdza poprawnosc znaku i zmniejsza jego kod o '1'. Konczy program jesli znak jest niepoprawny
; wejscie:      al - kod znaku
; wyjscie       al - zmniejszony o '1' kod znaku - sprowadzony do przedzialu <0,PERM_LEN-1>
_check_sign: 
    cmp         al, '1'+PERM_LEN-1      ; porownaj kod znaku z najwieszym mozliwym
    jg          _error                  ; jesli jest wiekszy niz 'Z' koncze program
    cmp         al, '1'                 ; porownuje znak z najmniejszym mozliwym
    jl          _error                  ; jesli jest mniejszy niz '1' koncze program
    sub         al, '1'                 ; kod sprowadz do przedzialu <0, PERM_LEM-1>
    ret

; Procedura sprawdzająca poprawność permutacji. Konczy program z sygnalem bledu
; jesli rozpatrywana permutacja jest niepoprawna
; wejscie:      rsi - adres tablicy z permutacja
;               bl - oczekiwana wartosc occur[x] dla znaku x przed wywolaniem procedury
;               bh - oczekiwana dlugosc parametru
; wyjscie:      rax - 0
;               rcx - dlugosc parametru
;               kazdy kod znaku w tablicy z permutacja zostaje zmniejszony o kod '1'         
_check_param:
    xor         rax, rax                ; wartosc calego rax bedzie rowna kodowi znaku
    xor         rcx, rcx                ; zeruj licznik znakow
    mov         dl, bl                  ; w dl bedzie 
    inc         dl                      ; oczekiwana liczba znakow+1
_check_param_loop:
    mov         al, [rsi]               ; wstaw do al kod znaku
    test        al, al                  ; czy aktualny znak to '\0'?
    jz          _check_param_end        ; jesli tak zakoncz petle
    call        _check_sign             ; sprawdz poprawnosc znaku
    mov         [rsi], al               ; wstaw spowrotem do bss zmniejszony kod znaku
    cmp         bl, PERM_LEN            ; czy sprawdzamy aktualnie permutacje? (tak jesli ocz. dl. par. = PERM_LEN)
    jne         _no_perm                ; jesli nie - nie sprawdzamy ilosci wystepowan znakow
    cmp         bl, [occur+rax]         ; porownaj iloc wystepowan znaku z oczekiwana iloscia
    jne         _error                  ; zakoncz program jesli znak wystepuje zla ilosc razy
    mov         [occur+rax], dl         ; wstaw do bss nowa ilosc wystepowan znaku - dana w dl = bl+1
_no_perm:
    inc         rsi                     ; przesun wskaznik na kolejny znak 
    inc         rcx                     ; zwieksz licznik znakow
    jmp         _check_param_loop       ; kontynuuj sprawdzanie parametru
_check_param_end:
    cmp         cl, bh                  ; porownaj z oczekiwana dlugoscia parametru
    jne         _error                  ; zakoncz jesli dlugosc jest inna
    ret

; Procedura sprawdzajaca poprawnosc permutacji T
; konczy program z sygnalem bledu jesli permutacja jest niepoprawna
; wyjscie:      rax - adres permutacji T
;               rcx - 0
;               bl, dl - smieci zalezne od przejscia algorytmu
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
; wejscie:      rax - kod znaku w przedziale <0,PERM_LEN-1>
;               bh - pozycja bebenka l w przedziale <0,PERM_LEN-1>
;               bl - pozycja bebenka r w przedziale <0,PERM_LEN-1>
; wyjscie       al - przepermutowany kod znaku w przedziale <0,PERM_LEN-1>
_parse_sign:
    inc         bl                      ; zwieksz pozycje bębenka r
; sprawdz czy bebenek r jest na ktorejs z pozycji obrotowych
    cmp         bl, 'L'-'1'             ; czy r jest na na L?
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

    ret
; Glowna procedura programu
_start:
    pop          rax                    ; zdejmij ze stosu liczbe argumentow
    cmp          rax, 5                 ; ilość argumentów na wejsciu powinna wynoscic 5
    jne          _error                 ; zakoncz program z sygnalem bledu w przeciwnym przypadku

    pop         rax                     ; pierwszy argument to nazwa pliku wykonywalnego

    popTo       [permL]                 ; drugi argument to adres do permutacji L
    popTo       [permR]                 ; trzeci to adres do permutacji R
    popTo       [permT]                 ; czwarty to adres do permutacji T

    checkParam  [permL], 0, PERM_LEN    ; sprawdz poprawnosc permutacji L
    checkParam  [permR], 1, PERM_LEN    ; sprawdz poprawnosc permutacji R
    checkParam  [permT], 2, PERM_LEN    ; sprawdz poprawnosc permutacji T

    createRev   [permL], permRevL       ; tworz odwrotnosc permutacji L - zapisz pod permRevL
    createRev   [permR], permRevR       ; tworz odwrotnosc permutacji R - zapisz pod permRevR

    call        _check_t                ; sprawdz poprawnosc permutacji T

    pop         r11                     ; zdejmij do r10 adres klucza
    checkParam  r11, 0, 2               ; sprawdz poprawnosc klucza
    mov         bx, [r11]               ; wstaw do bx klucz - w bh poczatkowa pozycja l - w bl poczatkowa pozycja r
_input_loop:
    mov         rax, SYS_READ           ; ustaw do wywołania funkcję sys_read
    xor         edi, edi                ; korzystaj ze standardowego wejscia - STDIN = 0
    mov         edx, BUFF_LEN           ; czytaj max BUFF_LEN znaków
    mov         rsi, buffer             ; ustaw adres czytania na adres bufora
    syscall                             ; wywołaj funkcję systemową
                      
    test        eax, eax                ; jesli wczytano 0 znakow
    jz          _input_end              ; skoncz wczytywanie

    mov         edx, eax                ; w dx trzymamy ilosc znakow do wypisania = ilosci wczytanch znakow
    xor         rcx, rcx                ; inicjalizuj licznik petli
_check_loop:
    mov         al, [buffer+rcx]        ; kopiuj aktualny znak z bufora
    call        _check_sign             ; czy rozpatrywany znak jest poprawny? 

    call        _parse_sign             ; koduj znak
    add         al, '1'                 ; zmien spowrotem na kod znaku do wypisania
    mov         [buffer+rcx], al        ; wstaw przepermutowany znak do bufora   
    inc         cx                      ; zwieksz licznik petli
    cmp         cx, dx                  ; czy zostaly przejrzane wszystkie znaki? 
    jne          _check_loop            ; jesli nie kontynuuj parsowanie bufora
 
    mov         eax, SYS_WRITE          ; ustaw do wywołania funkcję sys_write
    mov         edi, STDOUT             ; korzystaj ze standarowego wyjścia
    mov         esi, buffer             ; wypisuj z bufora
    syscall                             ; wywołaj funkcję systemową

    jmp         _input_loop             ; czytaj kolejny blok
_input_end:
    exit        0