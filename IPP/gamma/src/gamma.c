/** @file
 * Implementacja klasy przechowującej stan gry gamma
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 17.04.2020
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "gamma.h"
#include "utils.h"

/**
 * Struktura przechowująca stan gry.
 */
typedef struct gamma {
	uint32_t width;			/**< ilość kolumn planszy */
	uint32_t height;		/**< ilość wierszy planszy */
	uint32_t max_areas;		/**< maksymalna ilość obszarów gracza */
	uint32_t players;		/**< ilość graczy */
	uint32_t *areas;		/**< tablica przechowująca ilość obszarów graczy */
	uint32_t **board;		/**< dwuwymiarowa tablica przechowująca stan planszy */
	uint64_t *pawns;	/**< ilość pionków gracza na planszy */
	uint64_t all_pawns;	/**< ilość wszystkich pionków na planszy */
	bool *golden;		/**< tablica przechowująca informację 
 * czy gracz wykonał złoty ruch */
	uint64_t dfs_runs;		/**< ilość wykonanych przeszukiwań wgłąb */
	uint64_t **pre_time;	/**< tablica przechowująca numer ostatniego
 * wykonanego przejścia w głąb na polach planszy */
 	uint8_t **splits;	/**< ilość obszarów, na które dzieli pole */
 	uint64_t **low;		/**< wartość funkcji low dla pola */
	uint64_t *edges;	/**< tablica przechowująca ilość wolnych pól
 * graniczących z polami zajętymi przez graczy */
} gamma_t;

/** @brief Przesuwa współrzędne o jedno pole zgodnie z podanym kierunkiem.
 * Sprawdza czy przesunięcie współrzędnych w podanym kierunku jest legalne,
 * nowe współrzędne muszą znajdować się na obszarze planszy opisanej przez @p g.
 * Jeśli przesunięcie jest legalne wykonuje je.
 * @param[in] direction - kierunek zmiany współrzędnych.
 * @param[in, out] x    – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in, out] y    – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @param[in] g 		– wskaźnik na strukturę przechowującą stan gry.
 * @return Wartość @p true jeśli udało się przesunąć współrzędne
 * lub @p false w przeciwnym przypadku.
 */
static bool move_coord(const direction_m direction,
		uint32_t *x, uint32_t *y, gamma_t *g) {
	uint32_t x_new = *x;
	uint32_t y_new = *y;

	if (direction == UP && *y > 0) {
		--y_new;
	} else if (direction == DOWN && *y < g->height-1) {
		++y_new;
	} else if (direction == RIGHT && *x < g->width-1) {
		++x_new;
	} else if (direction == LEFT && *x > 0){
		--x_new;
	}

	if (x_new != *x || y_new != *y) {
		*x = x_new;
		*y = y_new;
		return true;
	}

	return false;
}

/** @brief Podaje ilość pól sąsiadujących ze wskazanym polem, na których
 * stoi pionek gracza @p player
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Ilość sąsiadujących pól zajętych przez wskazanego gracza.
 */
static uint32_t count_neighbors(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	assert(g);
	assert(x < g->width && y < g->height);
	assert(player > 0 && player <= g->players);

	uint32_t neighbors = 0;

	for (direction_m dir = UP; dir <= LEFT; ++dir) {
		uint32_t new_x = x;
		uint32_t new_y = y;

		if (move_coord(dir, &new_x, &new_y, g) &&
			g->board[new_y][new_x] == player) {
			++neighbors;
		};
	}
	return neighbors;
}

/** @brief Wykonuje algorytm przeszukiwania wgłąb oraz zaznacza punkty artykulacji.
 * Dla każdego pola z obszaru, do którego należy komórka o współrzędnych @p x @p y
 * ustawia w tablicy gamma::splits na ile obszarów podzieli się aktualny
 * po usunięciu gracza z danego pola.
 * @param[in, out] g  – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Ilość przeszukanych pól.
 */
static uint64_t dfs(gamma_t *g, uint32_t x,
		uint32_t y, uint64_t pre_time) {
	assert(g);
	assert(x < g->width && y < g->height);
	assert(g->board[y][x] != 0);

	g->pre_time[y][x] = pre_time;
	g->low[y][x] = pre_time;
	uint64_t graph_size = 1;
	uint8_t splits = 0;

	for (direction_m dir = UP; dir <= LEFT; ++dir) {
		uint32_t moved_x = x;
		uint32_t moved_y = y;

		if (move_coord(dir, &moved_x, &moved_y, g) &&
				g->board[moved_y][moved_x] == g->board[y][x]) {
			if (g->pre_time[moved_y][moved_x] <= g->dfs_runs) {
				graph_size += dfs(g, moved_x, moved_y, pre_time+1);

				if (g->low[moved_y][moved_x] >= g->pre_time[y][x]) {
					++splits;
				}
			}

			if (g->low[moved_y][moved_x] < g->low[y][x]) {
				g->low[y][x] = g->low[moved_y][moved_x];
			}
		}
	}

	g->splits[y][x] = splits+(pre_time > g->dfs_runs+1);

	return graph_size;
}

/** @brief Zaznacza punkty arytkulacji w obszarze wskazanego pola.
 * @param[in, out] g  – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
static void count_splits(gamma_t *g, uint32_t x, uint32_t y) {
	assert(x < g->width && y < g->height);
	assert(g->board[y][x] != 0);

	g->dfs_runs += dfs(g, x, y, g->dfs_runs+1);
}

gamma_t* gamma_new(uint32_t width, uint32_t height,
				   uint32_t players, uint32_t areas) {

	gamma_t *g = malloc(sizeof(*g));

	if (g == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	if (width == 0 || height == 0 || players == 0 || areas == 0) {
		free(g);
		errno = EINVAL;
		return NULL;
	}

	g->width = width;
	g->height = height;
	g->players = players;
	g->max_areas = areas;
	g->all_pawns = 0;
	g->dfs_runs = 0;

	g->areas = calloc(players, sizeof(*g->areas));
	g->golden = calloc(players, sizeof(*g->golden));
	g->pawns = calloc(players, sizeof(*g->pawns));
	g->board = calloc(height, sizeof(*g->board));
	g->splits = calloc(height, sizeof(*g->splits));
	g->low = calloc(height, sizeof(*g->low));
	g->pre_time = calloc(height, sizeof(*g->pre_time));
	g->edges = calloc(players, sizeof(*g->edges));

	uint32_t alloc_rows = 0;

	if (g->areas && g->board && g->golden && g->pawns &&
		g->edges && g->pre_time && g->splits && g->low) {
		for (;alloc_rows < height; ++alloc_rows) {
			g->board[alloc_rows] =
					calloc(width, sizeof(**g->board));
			g->pre_time[alloc_rows] =
					calloc(width, sizeof(**g->pre_time));
			g->splits[alloc_rows] =
					calloc(width, sizeof(**g->splits));
			g->low[alloc_rows] =
					calloc(width, sizeof(**g->low));
			if (!g->board[alloc_rows] || !g->pre_time[alloc_rows]) {
				free(g->board[alloc_rows]);
				free(g->pre_time[alloc_rows]);
				free(g->splits[alloc_rows]);
				free(g->low[alloc_rows]);
				break;
			}
		}
	}

	if (alloc_rows != height) {
		for (uint32_t i = 0; i < alloc_rows; ++i) {
			free(g->board[i]);
			free(g->pre_time[i]);
			free(g->splits[i]);
			free(g->low[i]);
		}
		free(g->golden);
		free(g->areas);
		free(g->board);
		free(g->pre_time);
		free(g->pawns);
		free(g->edges);
		free(g->low);
		free(g->splits);
		free(g);
		g = NULL;
		errno = ENOMEM;
	}

	return g;
}

void gamma_delete(gamma_t *g) {
	if (g) {
		for (uint32_t i = 0; i < g->height; ++i) {
			free(g->board[i]);
			free(g->pre_time[i]);
			free(g->splits[i]);
			free(g->low[i]);
		}
		free(g->pawns);
		free(g->golden);
		free(g->board);
		free(g->pre_time);
		free(g->low);
		free(g->splits);
		free(g->edges);
		free(g->areas);
		free(g);
	}
}

bool gamma_move(gamma_t *g, uint32_t player,
		uint32_t x, uint32_t y) {
	if (!g || x >= g->width || y >= g->height ||
		player == 0 || player > g->players) {
		errno = EINVAL;
		return false;
	}

	if (g->board[y][x] != 0 ||
		(g->areas[player-1] == g->max_areas &&
		count_neighbors(g, player, x, y) == 0)) {
		return false;
	}
	g->board[y][x] = player;
	count_splits(g, x, y);

	g->areas[player-1] -= g->splits[y][x]-1;
	++g->all_pawns;
 	++g->pawns[player-1];

	uint32_t neighbors[4];
	uint32_t various_neigh = 0;

	for (direction_m dir = UP; dir <= LEFT; ++dir) {
		uint32_t new_x = x;
		uint32_t new_y = y;

		if (move_coord(dir, &new_x, &new_y, g)) {
			if (g->board[new_y][new_x] == 0 &&
				count_neighbors(g, player, new_x, new_y) == 1) {
				++g->edges[player-1];
			} else if (g->board[new_y][new_x] != 0) {
				bool found = false;
				for (uint32_t i = 0; i < various_neigh; ++i) {
					if (neighbors[i] == g->board[new_y][new_x]) {
						found = true;
						break;
					}
				}

				if (!found) {
					assert(-g->edges[g->board[new_y][new_x]-1] > 0);
					--g->edges[g->board[new_y][new_x]-1];
					neighbors[various_neigh++] = g->board[new_y][new_x];
				}
			}
		}
	}

	return true;
}

bool gamma_golden_move(gamma_t *g, uint32_t player,
		uint32_t x, uint32_t y) {
	if (!g || x >= g->width || y >= g->height ||
		player == 0 || player > g->players) {
		errno = EINVAL;
		return false;
	}

	if  (g->golden[player-1] || !g->board[y][x] ||
			g->board[y][x] == player) {
		return false;
	}

	if (g->areas[player-1] == g->max_areas &&
		count_neighbors(g, player, x, y) == 0) {
		return false;
	}

	const size_t new_areas = g->splits[y][x]-1;
	const uint32_t prev_player = g->board[y][x];

	if (g->areas[g->board[y][x]-1]+new_areas > g->max_areas) {
		return false;
	}

	g->areas[prev_player-1] += new_areas;
	g->board[y][x] = player;

	count_splits(g, x, y);

	g->areas[player-1] -= g->splits[y][x]-1;

	for (direction_m dir = UP; dir <= LEFT; ++dir) {
		uint32_t new_x = x;
		uint32_t new_y = y;

		if (move_coord(dir, &new_x, &new_y, g) &&
				g->board[new_y][new_x] == 0) {
			if (count_neighbors(g, player, new_x, new_y) == 1) {
				++g->edges[player-1];
			}
			if (count_neighbors(g, prev_player, new_x, new_y) == 0) {
				assert(g->edges[prev_player-1] > 0);
				--g->edges[prev_player-1];
			}
		}
	}

	++g->pawns[player-1];
	--g->pawns[prev_player-1];
	g->golden[player-1] = true;

	return true;
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
	if (!g || player == 0 || player > g->players) {
		errno = EINVAL;
		return 0;
	}
	if (g->areas[player-1] != g->max_areas) {
		return (uint64_t) g->width * g->height - g->all_pawns;
	} else {
		return g->edges[player-1];
	}
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
	if (!g || player == 0 || player > g->players) {
		errno = EINVAL;
		return 0;
	}

	return g->pawns[player-1];
}

/** @brief Sprawdza czy gracz może wykonać złoty ruch na wskazanym polu.
 *
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartosść @p true jeśli gracz może wykonać ruch na wskazanym polu lub
 * @p false w przeciwnym przypadku.
 */
static bool gamma_golden_field(gamma_t *g, uint32_t player,
		uint32_t x, uint32_t y) {
	uint32_t f_player = g->board[y][x];

	if (f_player == 0 || f_player == player) {
		return false;
	}

	if (g->areas[f_player-1]+g->splits[y][x]-1 > g->max_areas) {
		return false;
	}

	if (g->areas[player-1] < g->max_areas ||
			count_neighbors(g, player, x, y) > 0) {
		return true;
	}

	return false;
}


bool gamma_golden_possible(gamma_t *g, uint32_t player) {
	if (!g || player == 0 || player > g->players) {
		errno = EINVAL;
		return false;
	}
	if (g->golden[player-1] ||
		g->all_pawns == g->pawns[player-1]) {
		return false;
	}

	for (uint32_t y = 0; y < g->height; ++y) {
		for (uint32_t x = 0; x < g->width; ++x) {
			if (gamma_golden_field(g, player, x, y)) {
				return true;
			}
		}
	}

	return false;
}


char *gamma_board(gamma_t *g) {
	if (!g) {
		errno = EINVAL;
		return NULL;
	}

	uint8_t col_len = number_length(g->players);

	if (col_len > 1) {
		++col_len;
	}

	uint64_t size = g->height*(col_len*g->width+1)+1;

	char *str_board = malloc(size);
	char *buff_it = str_board;

	if (str_board) {
		for (uint32_t y = 0; y < g->height; ++y) {
			for (uint32_t x = 0; x < g->width; ++x) {
				if (g->board[g->height-y-1][x] != 0) {
					sprintf(buff_it, "%*u", col_len,
							g->board[g->height-y-1][x]);
				} else {
					sprintf(buff_it, "%*c",
							col_len, '.');
				}
				buff_it += col_len;
			}
			(*buff_it++) = '\n';
		}
		(*buff_it) = 0;
	} else {
		errno = ENOMEM;
	}

	return str_board;
}

