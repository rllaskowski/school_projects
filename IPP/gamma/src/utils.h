/** @file
 * Interfejs funkcji pomocniczych.
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 10.06.2020
 */


#ifndef _UTILS_H_
#define _UTILS_H_

/**
 * Możliwe kierunki poruszania się z pola na pole
 */
typedef enum direction {
	UP,		/**< Góra */
	DOWN,	/**< Dół */
	RIGHT,	/**< Prawo */
	LEFT	/**< Lewo */
} direction_m;

/**
 * Kombinacje klawiszy pobrane z klawiatury
 */
typedef enum control {
	KEY_UP,		/**< Strzałka w górę */
	KEY_DOWN,	/**< Strzałka w dół */
	KEY_RIGHT,	/**< Strzałka w prawo */
	KEY_LEFT,	/**< Strzałka w lewo */
	KEY_C,		/**< Klawisz D */
	KEY_SPACE,	/**< Klawisz Spacja */
	KEY_G,		/**< Klawisz G */
	CTRL_D,		/**< Kombinacja klawiszy CTRL+D */
} control_m;

/** @brief Podaje ilość cyfr liczby w systemie 10.
 * @param[in] number  – liczba bez znaku
 * @return Ilość cyfr liczby w systemie 10
 */
uint8_t number_length(uint64_t number);

/** @brief Przełącza terminal na tryb raw.
 * Terminal w trybie raw nie oczekuje na wciśnięcie
 * klawisza Enter aby przeprocesować dane z wejścia.
 * @return Oryginalne ustawienia terminala
 */
struct termios start_raw_mode();

/** @brief Zmienia ustawienia terminala.
 * @param[in] mode   – Struktura opisująca ustawienia terminala
 */
void set_term_mode(struct termios mode);

/** @brief Czyści okno terminala **/
void scr_clear();

/** @brief Przesuwa kursor w podanym kierunku.
 * @param[in] dir	– Kierunek przesunięcia kursora
 * @param[in] shift	- Długość przesunięcia
 */
void cursor_move(direction_m dir, uint32_t shift);

/** @brief Przesuwa kursor w podanym kierunku.
 * @param[in] dir	– Kierunek przesunięcia kursora
 * @param[in] shift	- Długość przesunięcia
 */
void cursor_clear();

/** @brief Zwraca pobraną z klawiatury kombinację klawiszy.
 * Oczekuje na wpisanie kombinacji klawiszy wymienionej w @ref control.
 * Wymaga uprzedniego przełączenia terminala w tryb raw.
 * @return Zwraca pobraną kombinację klawiszy.
 */
control_m get_key();

/** @brief Parsuje napis na 32 bitową liczbę bez znaku.
 * @param[in] 	ui_str	– Wskaźnik na napis reprezentujący liczbę
 * @param[out] 	res		- Wskaźnik na liczbę przechowującą wynik parsowania
 * @return Wartość @p true gdy napis jest poprawną reprezentacją liczby
 * bez znaku lub @p false w przeciwnym przypadk.u
 */
bool parse_ui(char *ui_str, uint32_t *res);

#endif	/* _UTILS_H_ */