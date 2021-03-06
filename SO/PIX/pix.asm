extern pixtime
global pix

section .text

; Makro obliczajace {16^x % mod} korzystając z algorytmu szybkiego potęgowania
; input:        rcx - wykładnik potęgi
;               r9  - wartość mod
; output:       r11 - 16^(rcx) % (r9) 
%macro  power_mod 0
        xor         r11d, r11d          ; Inicjalizacja wyniku dla przypadku szczegolnego
        cmp         r9, 1               ; Przypadek szczegolny - wartosc modulo = 1. Wtedy wynik = 0
        je          %%power_mod_end
        inc         r11b                ; Inicjalizuj wynik potegowania - 1
        mov         eax, 16             ; Inicjalizuj rax jako pierwszą potęgę 16tki - 16^(2^0)
   
%%power_mod_loop:
        test        rcx, rcx            ; Czy wykładnik jest równy 0?
        jz          %%power_mod_end     ; Jeśli tak zakończ obliczenia

        test        cl, 1               ; Czy wykładnik jest parzysty?
        jz          %%even              ; Jeśli tak, pomiń mnożenie wyniku przez potęgę

        mov         r12, rax            ; Zachowaj tymczasowo aktualną potęgę
        mul         r11                 ; Mnóż wynik przez 16 w odpowiedniej potedze
        div         r9                  ; Działania wykonywane są modulo
        mov         r11, rdx            ; W rdx znajduje się reszta z dzielenia
        mov         rax, r12            ; Przywróć do rax aktualną potęgę 
%%even:              
        mul         rax                 ; Podnosi aktualną potęge 16 do kwadratu    
        div         r9                  ; Działania wykonywane są modulo
        mov         rax, rdx            ; w rdx jest reszta z dzielenia
    
        shr         rcx, 1              ; Dziel wykladnik przez 2
        jmp         %%power_mod_loop
%%power_mod_end:

%endmacro
     
%macro call_pixtime 0
        rdtsc                           ; Liczba cykli procesora znajduje sie teraz w edx;eax
        mov         edi, edx            ; Po kolei wstaw  
        shl         rax, 32             ; edx:eax
        shld        rdi, rax, 32        ; Do rdi
    
        call        pixtime             ; W rdi jest liczba cykli procesora
%endmacro

; Oblicza 8 pierwszych cyfr {S_j} ze wzoru:
; https://math.stackexchange.com/questions/880904/how-do-you-use-the-bbp-formula-to-calculate-the-nth-digit-of-%CF%80
; input:        r9  - j
;               r10 - n 
; output:       r8 - 8 pierwszych cyfr {S_j}          
_compute_frac: 
        xor         r8d, r8d            ; Na poczatku suma = 0
        xor         esi, esi            ; na poczatku k = 0
_compute_frac_loop:
        mov         rcx, r10            ; Inicjalizuj wykladnik potegowania 
        sub         rcx, rsi            ; n-k
    
        power_mod                       ; Oblicz 16^(n-k) % (8*k+j)

        mov         rdx, r11            ; Wynik potegowania jest w r11 - Przemnoz go przez 2^64
        xor         eax, eax            ; Przed dzieleniem nalez wyzerowac najmniej wazne bity
        div         r9                  ; Dziel przez (8*k+j)
        add         r8, rax             ; Zwieksz wynikowa sume

        add         r9, 8               ; Zwieksz wartosc (8*k+j) 
    
        inc         rsi                 ; Zwieksz k
        cmp         r10, rsi            ; Porownaj k z n
        jge         _compute_frac_loop  ; Jeśli k <= n licz dalej

        mov         rsi, 1<<60          ; {1/16}
_no_end_loop:
        xor         edx, edx            ; Wyzeruj najwzniejsze bity rdx:rax
        mov         rax, rsi            ; 
        div         r9                  ; Dziel przez {(1/16)^(k-n)} / (8*k+j)
        
        add         r8, rax             ; Zwieksz wynikowa sume

        shr         rsi, 4              ; {(1/16)^(k-n)}
        add         r9, 8               ; Zwieksz wartosc (8*k+j)

            
        test        rax, rax            ; Czy iloraz jest jeszcze istotny?
        jnz         _no_end_loop

        ret

; Oblicza 8 cyfr rozwiniecia liczby PI w systemie 16tkowym na pozycji 8*pixd. Zwieksza pixd o 1
; https://math.stackexchange.com/questions/880904/how-do-you-use-the-bbp-formula-to-calculate-the-nth-digit-of-%CF%80
; input:        rdi - &pix - adres tablicy do wstawienia wyniku
;               rsi - &pixd - adres zmiennej z indeksem w tablicy do wstawienia wyniku
;               rdx - max - maksymalna wartosc pixd
pix:
        push        r12                     ; Zachowaj rejestr r12 -- konwencja ABI
        push        r13                     ; Zachowaj rejestr r13 -- konwencja ABI
        push        r14                     ; Zachowaj rejestr r14 -- konwencja ABI
        push        r15                     ; Zachowaj rejestr r15 -- konwencja ABI
        mov         r14, rsi                ; Zachowaj &pixd
        mov         r15, rdx                ; Zachowaj wartosc max 
        push        rdi                     ; Zachowaj &ppi
        call_pixtime
        pop         rdi                     ; Przywroc &ppi
      
_loop:
        mov         r10, 1                  ; Wartosc pidx zostanie zwiekszona o 1
        lock        xadd QWORD[r14], r10    ; Pobierz z pamieci oraz zwieksz atomowo o 1 pidx

        cmp         r10, r15                ; Jeśli pobrane pidx jest wieksze od maksymalnej dozwolonej wartosci(max)
        jg          _end                    ; Przejdź do konca programu

        shl         r10, 3                  ; n = 8*m

        mov         r9d, 1                  ; j = 1
        call        _compute_frac           ; Oblicz S_1
        lea         r13, [r8*4]             ; Inicjalizuj wynik jako 4*S_1


        mov         r9d, 4                  ; j = 4
        call        _compute_frac           ; Oblicz S_4
        shl         r8, 1                   ; Mnoz S_4 przez 2
        sub         r13, r8                 ; Odejmuj od wyniku S_4*2

        mov         r9d, 5                  ; j = 5
        call        _compute_frac           ; Oblicz S_5
        sub         r13, r8                 ; Odejmuj od wyniku S_5

        mov         r9d, 6                  ; j = 6
        call        _compute_frac           ; Oblicz S_5
        sub         r13, r8                 ; Odejmuj od wyniku S_6

        shr         r13, 32                 ; Wynikowe 8 cyfr znajduje sie w najstarszych 32 bitach

        shr         r10, 3                  ; m = 1/8 *n
        mov         DWORD[rdi+4*r10], r13d  ; Wstaw do pamieci wynikowe 8 cyfr rozwiniecia pi

        jmp         _loop                   ; Licz dalej
_end:
        pop         r15                     ; Przywroc rejestr r14 -- konwencja ABI
        pop         r14                     ; Przywroc rejestr r15 -- konwencja ABI
        pop         r13                     ; Przywroc rejestr r12 -- konwencja ABI
        pop         r12                     ; Przywroc rejestr r13 -- konwencja ABI
      
        sub         rsp, 8                  ; Przed wejsciem do funkcji stos ma byc przesuniety o 16k+8
        call_pixtime
        add         rsp, 8                  ; Przywroc wskaznik stosu

        ret
    
    align 8