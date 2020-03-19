## Asynchroniczne C

### Wprowadzenie
Pula wątków to mechanizm pozwalający na uzyskanie współbieżnego wykonywanie wielu zadań w ramach jednego programu. W skład puli wchodzi pewna liczba wątków roboczych (ang. worker threads), czekających na pojawienie się pracy do wykonania.

Użycie puli wątków pozwala uniknąć ciągłego powoływania i czekania na zakończenie się wątku przy wykonywaniu krótkotrwałych zadań współbieżnych. Pozwala też na wykonywanie bardzo dużej liczby zadań niezależnie od siebie w sytuacji, gdy liczba dostępnych potoków przetwarzania jest ograniczona.

### Szczegółowy opis puli wątków
Interfejs puli wątków znajduje się w pliku [threadpool.h](threadpool.h)

Wywołanie `thread_pool_init` inicjuje argument wskazywany przez `pool` jako nową pulę, 
w której będzie funkcjonować `pool_size` wątków obsługujących zgłoszone do wykonania zadania. 
Za gospodarkę pamięcią wskazywaną przez `pool` odpowiada użytkownik biblioteki. 
Poprawność działania biblioteki jest gwarantowana tylko, jeśli każda pula stworzona przez `thread_pool_init` 
jest niszczona przez wywołanie `thread_pool_destroy` z argumentem reprezentującym tę pulę.

Wywołanie `defer(pool, runnable)` zleca puli wątków `pool` wykonanie zadania opisanego przez argument `runnable`, 
argumenty function są przekazywane przez wskaźnik `args`, w polu `argsz` znajduje się długość dostępnego do 
pisania i czytania buforu znajdującego się pod tym wskaźnikiem. Za zarządzanie pamięcią wskazywaną przez `args` 
odpowiada klient biblioteki.

Funkcja `function` powinna zostać obliczona przez wątek z puli `pool`; wywołanie `defer` może zablokować
wywołujący je wątek, ale jedynie na potrzeby rejestracji zlecenia: powrót z `defer` jest niezależny od powrotu 
z wykonania function przez pulę.

Zadania zlecone do wykonania przez `defer` powinny móc wykonywać się współbieżnie i na tyle niezależnie od siebie, na ile to możliwe.
Można ograniczyć liczbę współbieżnie wykonywanych zadań do rozmiaru puli. 
Pula w czasie swojego działania nie powinna powoływać więcej wątków niż określono parametrem `pool_size`. 
Utworzone wątki są utrzymywane aż do wywołania `thread_pool_destroy`.

### Szczegółowy opis mechanizmu obliczeń future
Interfejs mechanizmu future znajduje się w pliku [future.h](future.h)

Wywołanie 

```
int err = async(pool, future_value, callable)
``` 

inicjuje pamięć wskazywaną przez `future_value`. Za zarządanie tą pamięcią odpowiada użytkownik biblioteki. 
Na puli `pool` zlecane jest wykonanie `function` z argumentu `callable`. Funkcja function zwraca wskaźnik do wyniku. 
Użytkownik biblioteki powinien zadbać, żeby poprawnie ustawiła też rozmiar wyniku wykorzystując do tego celu trzeci argument typu `size_t*`.

Wołający może teraz:

Zaczekać na zakończenie wykonania funkcji `function` przez wywołanie:

```
void *result = await(future_value);
```

Za gospodarkę pamięcią wskazywaną przez wskaźnik result odpowiada użytkownik biblioteki (pamięć ta może zostać przekazana do funkcji function za pomocą jej argumentów lub w tej funkcji zaalokowana).

Zlecić jakiejś puli, niekoniecznie tej, która zainicjowała `future_value`, wywołanie innej funkcji na wyniku:
```
err = map(pool2, mapped_value, future_value, function2);
```
Programy, w których aktywnie działa jakaś pula wątków, powinny mieć automatycznie ustawioną obsługę sygnałów. Ta obsługa powinna zapewniać, że program po otrzymaniu sygnału (SIGINT) zablokuje możliwość dodawania nowych zadań do działających pul, dokończy wszystkie obliczenia zlecone dotąd działającym pulom, a następnie zniszczy działające pule.

### Opis programu macierz
Program macierz wczytuje ze standardowego wejścia dwie liczby `k` oraz `n`, każda w osobnym wierszu. 
Liczby te oznaczają odpowiednio liczbę wierszy oraz kolumn macierzy. Następnie program czyta `k*n` linijek z danymi, 
z których każda zawiera dwie, oddzielone spacją liczby: `v`, `t`. Liczba `v` umieszczona w linijce `i` (numerację linijek zaczynamy od 0) 
określa wartość macierzy z wiersza `floor(i/n)` (numerację kolumn i wierszy zaczynamy od 0) oraz kolumny `i mod n`. Liczba t to liczba milisekund,
jakie są potrzebne do obliczenia wartości `v`. Oto przykładowe poprawne dane wejściowe:
```
2
3
1 2
1 5
12 4
23 9
3 11
7 2
```
Takie dane wejściowe tworzą macierz od dwóch wierszach i trzech kolumnach:
```
|  1  1 12 |
| 23  3  7 |
```
Program wczytuje tak sformatowane wejście, 
a następnie za pomocą puli wątków zawierającej 4 wątki liczy sumy wierszy,
przy czym pojedyncze zadanie obliczeniowe powinno podaje w wyniku wartość pojedynczej komórki macierzy, odczekawszy liczbę milisekund, 
które zostały wczytane jako potrzebne do obliczenia tej wartości (np. zadanie obliczeniowe wyliczenia wartości 3 z macierzy powyżej powinno odczekiwać 11 milisekund).
Po obliczeniu należy wypisać sumy kolejnych wierszy na standardowe wyjście, po jednej sumie w wierszu. Dla przykładowej macierzy powyżej umieszczonej w pliku data1.dat wywołanie:
```
$ cat data1.dat | ./macierz
```
powoduje pojawienie się na wyjściu
```
14
33
```
