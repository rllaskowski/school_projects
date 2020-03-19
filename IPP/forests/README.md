## IPP - małe zadanie
Po uruchomieniu programu na świecie nie ma żadnego lasu, żadnego drzewa ani żadnego zwierzęcia. Program czyta dane ze standardowego wejścia i wykonuje niżej opisane polecenia. W tych poleceniach parametr las jest nazwą lasu, parametr drzewo jest nazwą drzewa, a parametr zwierzę jest nazwą zwierzęcia. Prawidłowe nazwy są ciągami niebiałych znaków rozszerzonego ASCII o kodach od 33 do 255. Nazwę prawidłowego polecenia i jego parametry oddzielają białe znaki (znaki ' ', '\t', '\v', '\f', '\r' w C o kodach ASCII odpowiednio 32, 9, 11, 12, 13). Każde prawidłowe polecenie pojawia się w osobnym wierszu zakończonym linuksowym znakiem przejścia do nowej linii (znak \n w C, kod ASCII 10). Białe znaki na początku i końcu wiersza należy ignorować.

- `ADD las` – Dodaje las.
- `ADD las drzewo` – Dodaje las i drzewo w lesie.
- `ADD las drzewo zwierzę` – Dodaje las, drzewo w lesie i zwierzę żyjące na tym drzewie.
Polecenie ADD dodaje tylko te byty, których brakuje. Ponowna próba dodania istniejącego już na świecie bytu jest ignorowana.

- `DEL` – Usuwa wszystkie lasy, wszystkie w nich drzewa i wszystkie zwierzęta żyjące na tych drzewach.
- `DEL las` – Usuwa las, wszystkie w nim drzewa i wszystkie zwierzęta żyjące na tych drzewach.
- `DEL las drzewo` – Usuwa drzewo w podanym lesie i wszystkie zwierzęta żyjące na tym drzewie.
- `DEL las drzewo zwierzę` – Usuwa zwierzę żyjące na danym drzewie w podanym lesie.
Polecenie `DEL` usuwa tylko te byty, które istnieją. Próba usunięcia nieistniejącego bytu jest ignorowana.

- `PRINT` – Wypisuje wszystkie lasy.
- `PRINT las` – Wypisuje wszystkie drzewa w podanym lesie.
- `PRINT las drzewo` – Wypisuje wszystkie zwierzęta żyjące na danym drzewie w podanym lesie.
Polecenie `PRINT` wypisuje nazwy bytów posortowane leksykograficznie rosnąco według rozszerzonych kodów ASCII, każdą nazwę w osobnym wierszu. Jeśli nie ma żadnego bytu do wypisania, to niczego nie wypisuje.

- `CHECK las` – Sprawdza, czy istnieje las o danej nazwie.
- `CHECK las drzewo` – Sprawdza, czy istnieje drzewo o danej nazwie w podanym lesie.
- `CHECK las drzewo zwierzę` – Sprawdza, czy na podanym drzewie w podanym lesie żyje zwierzę o danej nazwie.
W poleceniu `CHECK` można podać gwiazdkę jako wartość parametru z wyjątkiem ostatniego. Gwiazdka jest metaznakiem pasującym do dowolnej nazwy.

Ponadto:

Puste i składające się z samych białych znaków wiersze należy ignorować.
Wiersze rozpoczynające się znakiem # należy ignorować.
Informacje wypisywane przez program i obsługa błędów
Program kwituje poprawne wykonanie polecenia, wypisując informację na standardowe wyjście:

Dla poleceń `ADD` i `DEL` wiersz ze słowem `OK`.
Dla polecenia `CHECK` wiersz ze słowem `YES` lub `NO` zależnie od wyniku tego polecenia.
Każdy wiersz wyjścia powinien kończyć się linuksowym znakiem przejścia do nowej linii (znak `\n` w C, kod ASCII 10). Jest to jedyny biały znak, jaki może pojawić się na wyjściu.
Program wypisuje informacje o błędach na standardowe wyjście diagnostyczne:

Dla każdego błędnego wiersza, np. z powodu błędnej nazwy polecenia lub złej liczby parametrów, należy wypisać wiersz ze słowem `ERROR`, zakończony linuksowym znakiem końca linii (znak \n w C, kod ASCII 10). Jest to jedyny biały znak, jaki może pojawić się na wyjściu.
