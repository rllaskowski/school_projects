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

/**
 * 32 bitowa liczba bez znaku
 */
typedef uint32_t uint;

/**
 * Możliwe kierunki poruszania się z pola na pole
 */
typedef enum direction {
	Up,		/**< góra */
	Right,	/**< prawo */
	Down,	/**< dół */
	Left	/**< lewo */
} direction_m;

/**
 * Struktura przechowująca stan gry.
 */
typedef struct gamma {
	uint width;			/**< ilość kolumn planszy */
	uint height;		/**< ilość wierszy planszy */
	uint max_areas;		/**< maksymalna ilość obszarów gracza */
	uint players;		/**< ilość graczy */
	uint *areas;		/**< tablica przechowująca ilość obszarów graczy */
	uint **board;		/**< dwuwymiarowa tablica przechowująca stan planszy */
	uint64_t *pawns;	/**< ilość pionków gracza na planszy */
	uint64_t all_pawns;	/**< ilość wszystkich pionków na planszy */
	bool *golden;		/**< tablica przechowująca informację 
 * czy gracz wykonał złoty ruch */
	uint dfs_runs;		/**< ilość wykonanych przeszukiwań wgłąb */
	uint **dfs_search;	/**< tablica przechowująca numer ostatniego
 * wykonanego przejścia w głąb na polach planszy */
	uint64_t *edges;	/**< tablica przechowująca ilość wolnych pól
 * graniczących z polami zajętymi przez graczy */
} gamma_t;

/** @brief Podaje ilość cyfr liczby w systemie 10
 * @param[in] number  – nieujemna liczba
 * @return Ilość cyfr liczby w systemie 10
 */
uint number_length(uint number) {
	if (number == 0) {
		return 1;
	}

	uint length = 0;

	while (number != 0) {
		++length;
		number /= 10;
	}

	return length;
}

/** @brief Przesuwa współrzędne o jedno pole zgodnie z podanym kierunkiem
 * @param[in] direction - kierunek zmiany współrzędnych.
 * @param[in, out] x    – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in, out] y    – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
void move_coord(const direction_m direction, uint *x, uint *y) {
	assert(direction >= Up && direction <= Left);

	*x += (direction % 2)*(direction < Down? 1  : -1);
	*y += ((direction+1) % 2)*(direction < Down? 1  : -1);
}

/** @brief Podaje liczbę pól sąsiadujących ze wskazanym polem, na których
 * stoi pionek gracza @p player
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
uint count_neighbors(gamma_t *g, uint player, uint x, uint y) {
	assert(g);
	assert(x > 0 && y > 0 && x <= g->width && y <= g->height);

	uint neighbors = 0;

	for (direction_m dir = Up; dir <= Left; ++dir) {
		uint new_x = x;
		uint new_y = y;
		move_coord(dir, &new_x, &new_y);

		if (g->board[new_y][new_x] == player) {
			++neighbors;
		};
	}
	return neighbors;
}

/** @brief Wykonuje algorytm przeszukiwania wgłąb zaznaczając numer przejścia
 * w tablicy @ref gamma::dfs_search struktury wskazanej przez @p g
 * @param[in, out] g  – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
void dfs(gamma_t *g, uint x, uint y) {
	assert(g);
	assert(x > 0 && y > 0 && x <= g->width && y <= g->height);
	assert(g->board[y][x] != 0);

	g->dfs_search[y][x] = g->dfs_runs;

	for (direction_m dir = Up; dir <= Left; ++dir) {
		uint moved_x = x;
		uint moved_y = y;

		move_coord(dir, &moved_x, &moved_y);

		if (g->board[moved_y][moved_x] == g->board[y][x] &&
			g->dfs_search[moved_y][moved_x] != g->dfs_runs) {
			dfs(g, moved_x, moved_y);
		}
	}
}

/** @brief Podaje liczbę obszarów, na które zostanie podzielony obszar zawierający
 * pionek na wskazanej pozycji. Na podanych współrzędnych musi stać pionek.
 * @param[in, out] g  – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       – numer kolumny, liczba większa od zera, nie większa od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba większa od zera, nie większa od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Liczba obszarów, na które zostanie podzielony obszar zawierający
 * pionek na wskazanej pozycji
 */
uint count_splits(gamma_t *g, uint x, uint y) {
	assert(x > 0 && y > 0 && x <= g->width && y <= g->height);
	assert(g->board[y][x] != 0);

	uint first_run = g->dfs_runs+1;
	uint player = g->board[y][x];
	g->board[y][x] = 0;

	uint areas = 0;

	for (direction_m dir = Up; dir <= Left; ++dir) {
		uint new_x = x;
		uint new_y = y;
		move_coord(dir, &new_x, &new_y);

		if (g->board[new_y][new_x] == player &&
			g->dfs_search[new_y][new_x] < first_run) {
			++g->dfs_runs;
			++areas;
			dfs(g, new_x, new_y);
		}
	}

	g->board[y][x] = player;

	return areas;
}

gamma_t* gamma_new(uint width, uint height,
				   uint players, uint areas) {

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

	g->areas = malloc((players + 1) * sizeof(*g->areas));
	g->golden = malloc((players+1)*sizeof(*g->golden));
	g->pawns = malloc((players + 1) * sizeof(*g->pawns));
	g->board = malloc((height+2)*sizeof(*g->board));
	g->dfs_search = malloc((height+2)*sizeof(*g->board));
	g->edges = malloc((players + 1) * sizeof(*g->edges));

	uint allocated_rows = 0;

	if (g->areas && g->board && g->golden && g->pawns &&
		g->edges && g->dfs_search) {
		for (uint i = 0; i <= players; ++i) {
			g->areas[i] = 0;
			g->pawns[i] = 0;
			g->golden[i] = false;
			g->edges[i] = 0;
		}

		for (;allocated_rows < height+2; ++allocated_rows) {
			g->board[allocated_rows] = malloc((width+2)*sizeof(*g->board[allocated_rows]));
			g->dfs_search[allocated_rows] = malloc((width+2)*sizeof(*g->board[allocated_rows]));

			if (!g->board[allocated_rows] || !g->dfs_search[allocated_rows]) {
				free(g->board[allocated_rows]);
				free(g->dfs_search[allocated_rows]);
				break;
			}

			for (uint i = 0; i < width+2; ++i) {
				g->board[allocated_rows][i] = 0;
				g->dfs_search[allocated_rows][i] = 0;
			}
		}
	}

	if (allocated_rows != height+2) {
		for (uint i = 0; i < allocated_rows; ++i) {
			free(g->board[i]);
			free(g->dfs_search[i]);
		}
		free(g->golden);
		free(g->areas);
		free(g->board);
		free(g->dfs_search);
		free(g->pawns);
		free(g->golden);
		free(g);
		g = NULL;
		errno = ENOMEM;
	}

	return g;
}

void gamma_delete(gamma_t *g) {
	if (g) {
		printf("%lu\n", sizeof(*g));
		for (uint i = 0; i <= g->height+1; ++i) {
			free(g->board[i]);
			free(g->dfs_search[i]);
		}
		free(g->pawns);
		free(g->golden);
		free(g->board);
		free(g->dfs_search);
		free(g->edges);
		free(g->areas);
		free(g);
	}
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	++x;
	++y;

	if (!g || x > g->width || y > g->height || player > g->players) {
		errno = EINVAL;
		return false;
	}

	if (g->board[y][x] != 0 ||
		(g->areas[player] == g->max_areas && count_neighbors(g, player, x, y) == 0)) {
		return false;
	}

	g->board[y][x] = player;
	g->areas[player] -= count_splits(g, x, y)-1;
	++g->all_pawns;
 	++g->pawns[player];

	uint neighbors[4];
	uint various_neigh = 0;

	for (direction_m dir = Up; dir <= Left; ++dir) {
		uint new_x = x;
		uint new_y = y;
		move_coord(dir, &new_x, &new_y);

		if (new_x != 0 && new_x <= g->width && new_y != 0 && new_y <= g->height) {
			if (g->board[new_y][new_x] == 0 && count_neighbors(g, player, new_x, new_y) == 1) {
				++g->edges[player];
			} else if (g->board[new_y][new_x] != 0) {
				bool found = false;
				for (uint i = 0; i < various_neigh; ++i) {
					if (neighbors[i] == g->board[new_y][new_x]) {
						found = true;
						break;
					}
				}

				if (!found) {
					--g->edges[g->board[new_y][new_x]];
					neighbors[various_neigh++] = g->board[new_y][new_x];
				}
			}
		}
	}

	return true;
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	++x;
	++y;

	if (!g || x > g->width || y > g->height || player > g->players) {
		errno = EINVAL;
		return false;
	}

	if  (g->golden[player] || !g->board[y][x] || g->board[y][x] == player) {
		return false;
	}

	if (g->areas[player] == g->max_areas && count_neighbors(g, player, x, y) == 0) {
		return false;
	}

	uint areas = count_splits(g, x, y);
	const uint prev_player = g->board[y][x];

	if (g->areas[g->board[y][x]]+areas-1 > g->max_areas) {
		return false;
	}

	g->areas[prev_player] += areas-1;

	g->board[y][x] = player;
	areas = count_splits(g, x, y);
	g->areas[player] -= areas-1;

	for (direction_m dir = Up; dir <= Left; ++dir) {
		uint new_x = x;
		uint new_y = y;
		move_coord(dir, &new_x, &new_y);

		if (new_x != 0 && new_x <= g->width && new_y != 0 && new_y <= g->height &&
				g->board[new_y][new_x] == 0) {
			if (count_neighbors(g, player, new_x, new_y) == 1) {
				++g->edges[player];
			}
			if (count_neighbors(g, prev_player, new_x, new_y) == 0) {
				--g->edges[prev_player];
			}
		}
	}

	++g->pawns[player];
	--g->pawns[prev_player];
	g->golden[player] = true;

	return true;
}

uint64_t gamma_free_fields(gamma_t *g, uint player) {
	if (!g || player > g->players) {
		errno = EINVAL;
		return 0;
	}
	if (g->areas[player] != g->max_areas) {
		return (uint64_t) g->width * g->height - g->all_pawns;
	} else {
		return g->edges[player];
	}
}

uint64_t gamma_busy_fields(gamma_t *g, uint player) {
	if (!g || player > g->players) {
		errno = EINVAL;
		return 0;
	}

	return g->pawns[player];
}


bool gamma_golden_possible(gamma_t *g, uint32_t player) {
	if (!g || player > g->players) {
		errno = EINVAL;
		return false;
	}
	if (g->golden[player]) {
		return false;
	}
	return g->all_pawns > g->pawns[player];
}


char *gamma_board(gamma_t *g) {
	if (!g) {
		errno = EINVAL;
		return NULL;
	}

	uint64_t size = g->height;

	for (uint y = 1; y <= g->height; ++y) {
		for (uint x = 1; x <= g->height; ++x) {
			uint length = number_length(g->board[y][x]);
			size += length + (length > 1? 2 : 0);
		}
	}

	char *str_board = malloc(size+1);

	uint64_t it = 0;

	if (str_board) {
		for (uint y = g->height; y >= 1; --y) {
			for (uint x = 1; x <= g->height; ++x) {
				if (g->board[y][x] != 0) {
					uint length = number_length(g->board[y][x]);

					if (length == 1) {
						str_board[it++] = '0'+g->board[y][x];
					} else {
						sprintf(str_board+it, "[%d]", g->board[y][x]);
						it += length+2;
					}

				} else {
					str_board[it++] = '.';
				}
			}
			str_board[it++] = '\n';
		}
		str_board[it] = 0;
	} else {
		errno = ENOMEM;
	}
	return str_board;
}
