## Prastarzy Szamani

### Wprowadzenie

Klan Prastarych Szamanów jest u kresu swego życia. Jedynym ratunkiem jest
przygotowanie eliksiru życia, który da im nieśmiertelność, jednak muszą
to zrobić szybko, gdyż ich dni są policzone. Produkcja owego eliksiru to
jednak długa przygoda, wymagająca zdobycia znacznej ilości smoczych jaj
oraz magicznych kryształów.

Szamani zaczynają swoją przygodę w smoczej jamie pełnej smoczych jaj. Jaja
różnią się rozmiarem oraz wagą. Worek bez dna jest magicznym przedmiotem
pozwalającym na zabranie jaj z pieczary, jednak jak się okazało ma on swój
limit pojemności. Aby przygotować eliksir życia szamani muszą wziąć tak dużo
jaj (w sensie wagi) jak to tylko możliwe (tzn. ile tylko zmieści się do worka).

Następnie szamani muszą dotrzeć do alchemika, właściciela kryształowych grot.
Alchemik chętnie podzieli się z szamanami jego kolekcją magicznych kryształów,
jednak nie zrobi tego za darmo - w zamian oczekuje, że szamani uporządkują
piasek na jego pustyni - układając go od ziarenek najmniejszych do największych.

Do stworzenia eliksiru życia wystarczy jeden magiczny kryształ, jednak musi on
być najwyższej możliwej jakości. W przeciwnym razie mikstura się nie uda, a
wszystkie smocze jaja będą do wyrzucenia. Dlatego szamani muszą dokładnie
porównać wszystkie kryształy w celu wybrania tego najlepszego.


### Zadanie
Celem zadania jest zaimplementowanie w C++ algorytmów w sposób sekwencyjny oraz 
współbieżnych trzech algorytmów, rozwiązujących następujące problemy:

- Dyskretny problem plecakowy (problem 1.)
- Sortowanie przez scalanie (Merge sort) lub sortowanie szybkie (Quicksort) (problem 2.)
- Znajdowanie elementu maksymalnego (problem 3.)

Implementacje algorytmów muszą znajdować prawidłowe rozwiązanie (a nie jego
przybliżenie). Implementacje sekwencyjne powinny być zaimplementowanie wydajnie:

- Problem 1. przy użyciu programowania dynamicznego
- Problem 2. w czasie O(NlogN)
- Problem 3. w czasie O(N)

Implementacje współbieżne powinny osiągać praktyczne przyspieszenie
(to znaczy działać szybciej niż ich wersje sekwencyjne dla  odpowiednio dużych danych
wejściowych), przy czym osiągnięte przyspieszenia będą różne dla poszczególnych algorytmów 
i różnych danych wejściowych. Celem zadania nie jest implementacja najszybszych współbieżnych 
algorytmów dla powyższych problemów opublikowanych w pracach naukowych, tylko wymyślenie 
własnych praktycznych modyfikacji oraz analiza ich wydajności.

## Rozwiązanie

Implementacja problemów podanych w zadaniu znajduje się w pliku [adventure.h](src/adventure.h)