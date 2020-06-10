/** @file
 * Implementacja gry gamma w trybie wsadowym i interaktywnym
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 17.05.2020
 */

/** Do korzystania z getline */
#define  _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>

#include "gamma.h"
#include "utils.h"

/** Białe znaki */
#define WS " \f\v\r\t"

typedef enum gamemode {
	I_MODE,	/**< Tryb interaktywny */
	B_MODE	/**< Tryb wsadowy */
} gamemode_m;

/**
 * Struktura opisująca parametry gry
 */
typedef struct game {
	gamemode_m mode;	/**< Tryb gry */
	uint32_t width;		/**< Szerokość planszy */
	uint32_t height;	/**< Wysokość planszy */
	uint32_t players;	/**< Ilość graczy */
	uint32_t areas;		/**< Maksymalna ilość obszarów gracza */
} game_t;

/** @brief Podaje parametry gry z podanego opisu.
 * @param[in] 	game_desc	- wskaźnik na napis reprezentujący opis gry
 * @param[out]	game	- wskaźnik na strukturę przechowującą parametry gry
 * @return Wartość @p true jeśli podany napis był poprawnym opisem gry.
 */
static bool parse_game_desc(char *game_desc, game_t *game) {
	char *mode = strtok(game_desc, WS);

	/* Niewłaściwa długość napisu opisującego rodzaj gry
	 * lub nie znajduje się on na początku linii */
	if (mode == NULL || strlen(mode) != 1 ||
		mode != game_desc) {
		return false;
	}

	if (mode[0] == 'B') {
		game->mode = B_MODE;
	} else if (mode[0] == 'I') {
		game->mode = I_MODE;
	} else {
		return false;
	}

	if (!parse_ui(strtok(NULL, WS), &game->width)) {
		return false;
	}
	if (!parse_ui(strtok(NULL, WS), &game->height)) {
		return false;
	}
	if (!parse_ui(strtok(NULL, WS), &game->players)) {
		return false;
	}
	if (!parse_ui(strtok(NULL, WS), &game->areas)) {
		return false;
	}

	/* Za dużo argumentów */
	if (strtok(NULL, WS) != NULL) {
		return false;
	}

	return true;
}
/** @brief Wykonuje komendę w trybie wsadowym gry.
 * @param[in, out] g		- wskaźnik na strukturę przechowującą
 * 							stan gry
 * @param[in] command		- wskaźnik na napis reprezentujący komendę
 * @return Wartość @p true jeśli podano poprawną komendę
 * lub @p false w przeciwnym przypadku.
 */
static bool run_command(gamma_t *g, char *command) {
	char *cmd_type = strtok(command, WS);

	/* Niewłaściwa długość napisu opisującego typ komendy
	 * lub nie znajduje się on na początku linii */
	if (cmd_type == NULL || strlen(cmd_type) != 1 ||
		command != cmd_type) {
		return false;
	}

	uint32_t args[3];
	uint8_t argz = 0;
	char *arg;

	while (argz < 3 && (arg = strtok(NULL, WS))) {
		/* Skoro argument został podany, to musi być poprawny... */
		if (!parse_ui(arg, &args[argz++])) {
			return false;
		}
	}

	/* Za dużo argumentów - max 3 */
	if (strtok(NULL, WS) != NULL) {
		return false;
	}

	if (cmd_type[0] == 'm') {
		if (argz != 3) {
			return false;
		}

		printf("%d\n", gamma_move(g, args[0], args[1], args[2]));
	} else if (cmd_type[0] == 'g') {
		if (argz != 3) {
			return false;
		}

		printf("%d\n", gamma_golden_move(g, args[0], args[1], args[2]));
	} else if (cmd_type[0] == 'b') {
		if (argz != 1) {
			return false;
		}

		printf("%lu\n", gamma_busy_fields(g, args[0]));
	} else if (cmd_type[0] == 'f') {
		if (argz != 1) {
			return false;
		}

		printf("%lu\n", gamma_free_fields(g, args[0]));
	} else if (cmd_type[0] == 'q') {
		if (argz != 1) {
			return false;
		}

		printf("%d\n", gamma_golden_possible(g, args[0]));
	} else if (cmd_type[0] == 'p') {
		if (argz != 0) {
			return false;
		}
		char *board = gamma_board(g);
		if (board) {
			printf("%s", board);
			free(board);
		} else {
			return false;
		}
	} else {
		return false;
	}

	return true;
}
/** @brief Rozpoczyna grę w trybie wsadowym.
 * @param[in, out] g			- wskaźnik na strukturę przechowującą stan gry
 * @param[in, out] buffer		- wskaźnik na bufor do wczytywania
 * 								ze standardowego wejścia
 * @param[in, out] buffer_size	- wskaźnik na wielkość bufora
 * @param[in, out] line_count	- wskaźnik na ilość wczytanych linii
 */
static void play_batch(gamma_t *g, char *buffer, size_t *buffer_size, size_t *line_count) {
	int32_t read;

	/* Czytaj aż do końca danych na wejściu */
	while ((read = getline(&buffer, buffer_size, stdin)) > 0) {
		if (strnlen(buffer, read) < (size_t)read) {
			/* W pobranej linii jest znak terminujący '\0' */
			fprintf(stderr, "ERROR %lu\n", *line_count);
		} else if (buffer[0] != '#' && buffer[0] != '\n') {
			if (buffer[read-1] == '\n') {
				/* Usuń znak nowej linii z końca buforu */
				buffer[--read] = 0;
			}

			if (!run_command(g, buffer)) {
				fprintf(stderr, "ERROR %lu\n", *line_count);
			}
		}

		++(*line_count);
	}
}

/** @brief Wskazuje następnego gracza, który może wykonać legalny ruch.
 * Podaje liczbę możliwych do wykonania przez niego ruchów oraz czy gracz ma
 * możliwość wykonania ruchu specjalnego.
 * @param[in, out] player	- wskaźnik na numer gracza
 * @param[in] game	- wskaźnik na strukturę przechowującą parametry gry
 * @param[in] g	- wskaźnik na strukturę przechowującą stan gry
 * @param[out] moves_possible	- wskaźnik na ilość możliwych ruchów gracza
 * @param[out] golden_possible	- wskaźnik na flagę możliwości wykonania
 * 								ruchu specjalnego
 * @return Wartość @p true jeśli istnieje gracz z legalnym ruchem
 * lub @p false w przeciwnym przypadku.
 */
static bool next_player(uint32_t *player, game_t *game, gamma_t *g,
							uint64_t *moves_possible, bool *golden_possible) {
	uint32_t search_count = 0;

	(*golden_possible) = false;
	(*moves_possible) = 0;

	while (!(*golden_possible) && (*moves_possible) == 0 &&
		   search_count < game->players) {
		(*player) = (*player)%game->players+1;

		if (gamma_golden_possible(g, *player)) {
			(*golden_possible) = true;
		}
		(*moves_possible) = gamma_free_fields(g, *player);

		++search_count;
	}
	/* Kończymy jeśli wszyscy gracze nie mogą wykonać ruchu */
	if (search_count == game->players && !(*golden_possible) &&
			(*moves_possible) == 0) {
		return false;
	}

	return true;
}

/** @brief Rozpoczyna grę w trybie interaktywnym.
 * @param[in, out] g	- wskaźnik na strukturę przechowującą stan gry
 * @param[in] game		- wskaźnik na struktruę przechowującą parametry gry
 */

static void play_interactive(gamma_t *g, game_t *game) {
	uint32_t player = 0;
	uint8_t col_len = number_length(game->players);

	if (col_len > 1) {
		++col_len;
	}
	/* Zachowanie oryginalego stanu terminala pozwoli na przywrócenie
	 * go po zakończeniu pracy w trybie raw */
	struct termios original = start_raw_mode();

	/* Coś innego niż CTRL_D */
	control_m control = KEY_UP;

	uint32_t cursor_x = 0;
	uint32_t cursor_y = game->height-1;

	bool golden_possible;
	uint64_t moves_possible;

	char *board;

	while (next_player(&player, game, g,
			&moves_possible, &golden_possible) && control != CTRL_D) {
		scr_clear();

		board = gamma_board(g);
		if (!board) {
			fprintf(stderr, "Brak pamięci!\n");
			exit(1);
		}
		printf("%s", board);
		printf("PLAYER %u %lu %lu", player,
				gamma_busy_fields(g, player), moves_possible);

		if (golden_possible) {
			printf(" G");
		}

		free(board);

		cursor_clear();

		cursor_move(RIGHT, cursor_x*col_len+col_len-1);
		cursor_move(DOWN, game->height-cursor_y-1);

		while (true) {
			while ((control = get_key()) <= KEY_LEFT) {
				if (control == KEY_UP && cursor_y+1 < game->height) {
					cursor_move(UP, 1);
					++cursor_y;
				} else if (control == KEY_RIGHT && cursor_x+1 < game->width) {
					cursor_move(RIGHT, col_len);
					++cursor_x;
				} else if (control == KEY_DOWN && cursor_y > 0) {
					cursor_move(DOWN, 1);
					--cursor_y;
				} else if (control == KEY_LEFT && cursor_x > 0) {
					cursor_move(LEFT, col_len);
					--cursor_x;
				}
			}

			if (control == CTRL_D) {
				break;
			} else if (control == KEY_SPACE)  {
				if (moves_possible &&
						gamma_move(g, player, cursor_x, cursor_y)) {
					break;
				}
			} else if (control == KEY_G)  {
				if (golden_possible &&
						gamma_golden_move(g, player, cursor_x, cursor_y)) {
					break;
				}
			} else if (control == KEY_C) {
				break;
			}
		}
	}

	scr_clear();

	board = gamma_board(g);
	if (!board) {
		fprintf(stderr, "Brak pamięci!\n");
		exit(1);
	}

	printf("%s", board);
	free(board);

	for (uint32_t i = 1; i <= game->players; ++i) {
		printf("PLAYER %*u %lu\n", col_len, i,
				gamma_busy_fields(g, i));
	}

	/* Przywróć oryginalne ustawienia terminala */
	set_term_mode(original);
}

/** @brief Główna funkcja programu */
int main() {
	size_t buffer_size;
	int32_t read;
	char *buffer = NULL;

	size_t line_count = 1;

	game_t game;
	gamma_t *gamma = NULL;

	/* Czytaj komendy aż do pierwszego poprawnego opisu gry
	 * lub końca wejścia */
	while ((read = getline(&buffer, &buffer_size, stdin)) > 0) {
		if (strnlen(buffer, read) < (size_t)read) {
			/* W pobranej linii jest znak terminujący '\0' */
			fprintf(stderr, "ERROR %lu\n", line_count);
		} else if (buffer[0] != '#' && buffer[0] != '\n') {
			/* Pomiń komentarze oraz puste linie */
			if (buffer[read-1] == '\n') {
				/* Usuń znak nowej linii z końca buforu */
				buffer[--read] = 0;
			}

			if (parse_game_desc(buffer, &game)) {
				gamma = gamma_new(game.width, game.height,
							game.players, game.areas);
			}

			if (gamma) {
				break;
			} else {
				fprintf(stderr, "ERROR %lu\n", line_count);
			}
		}

		++line_count;
	}

	if (gamma) {
		if (game.mode == I_MODE) {
			play_interactive(gamma, &game);
		} else {
			printf("OK %lu\n", line_count);

			++line_count;
			play_batch(gamma, buffer, &buffer_size, &(line_count));
		}

		gamma_delete(gamma);
	}

	free(buffer);

	return 0;
}

