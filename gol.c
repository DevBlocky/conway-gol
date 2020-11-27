#include "gol.h"

#include <stdlib.h>
#include <memory.h>

// PRIVATE
// Converts a 2d coordinate to a 1d coordinate using the game's length and width
// This function doesn't return any errors, instead returns the 1d coordinate
static inline unsigned int gol_2dto1d(struct gameoflife *game, const gol_pos x, const gol_pos y) {
    return (y * game->cols) + x;
}

// PRIVATE
// Provides the number of live neighbors (in a 3x3 around the cell position) in the count parameter
// Possbile Errors (return value): GOL_ERR_INIT, GOL_ERR_OK
static int gol_countlive(struct gameoflife *game, gol_pos x, gol_pos y, int *count) {
    // all possible relative positions of neighbors (x, y)
    const static gol_pos neighbors[][2] = {
            {-1, -1},
            {-1, 0},
            {-1, 1},
            {0,  -1},
            {0,  1},
            {1,  -1},
            {1,  0},
            {1,  1},
    };

    if (game->board == NULL) return GOL_ERR_INIT;

    // the kept count of live neighbors
    int n = 0;
    // go through all pairs of neighbors
    for (int i = 0; i < sizeof(neighbors) / sizeof(gol_pos[2]); i++) {
        // find the positions of the neighbor
        gol_pos nx = x + neighbors[i][0];
        gol_pos ny = y + neighbors[i][1];

        // if this exceeds the bounds of the board, continue
        if (nx < 0 || nx >= game->cols || ny < 0 || ny >= game->rows) continue;

        n += game->board[gol_2dto1d(game, nx, ny)];
    }

    *count = n;
    return GOL_ERR_OK;
}

// Initializes the gameoflife struct pointer with the length and width provided
// Assumes that the gameoflife struct pointer is already initialized, however allocated memory for the internal board pointer
// Possible errors (return value): GOL_ERR_NOMEM, GOL_ERR_OK
gol_err gol_init(struct gameoflife *game, gol_pos rows, gol_pos cols) {
    // create board in memory
    size_t sz = (size_t)rows * cols;
    bool *board = (bool *) calloc(sz, sizeof(bool));
    if (board == NULL) return GOL_ERR_NOMEM;

    // set properties
    game->rows = rows;
    game->cols = cols;
    game->board = board;

    // no other errors to report, and we're done
    return GOL_ERR_OK;
}

// Frees all memory associated with the gameoflife struct and resets the internal values
// This does not free the gameoflife struct itself, since it can be contained on the stack
// Possible errors (return value): GOL_ERR_OK
gol_err gol_free(struct gameoflife *game) {
    // free game board if it exists
    if (game->board)
        free(game->board);
    // reset values inside structure
    memset(game, 0, sizeof(struct gameoflife));

    return GOL_ERR_OK;
}

// Copies an instance of one gameoflife struct (src) into another (dest)
// This also allocates a completely new board, so the two structs won't be using the same board memory segment
// Possible errors (return value): GOL_ERR_NOMEM, GOL_ERR_OK
gol_err gol_copy(struct gameoflife *dest, const struct gameoflife *src) {
    // copy over the src internals of the structure to the destination
    memcpy(dest, src, sizeof(struct gameoflife));

    // create a new board pointer since it's would be bad if they both used
    // the same segment of memory for their operations
    if (src->board != NULL) {
        size_t board_len = (size_t)src->cols * src->rows;
        bool *board_copy = (bool *) malloc(board_len * sizeof(bool));
        if (board_copy == NULL) return GOL_ERR_NOMEM;
        memcpy(board_copy, src->board, board_len * sizeof(bool));
        dest->board = board_copy;
    }

    return GOL_ERR_OK;
}

// Populates the gameoflife struct board with random values (either 1 or 0) at each position
// Possible errors (return value): GOL_ERR_INIT, GOL_ERR_OK
gol_err gol_populate(struct gameoflife *game) {
    if (game->board == NULL) return GOL_ERR_INIT;

    // loop through all possible coordinates
    for (gol_pos y = 0; y < game->rows; y++) {
        for (gol_pos x = 0; x < game->cols; x++) {
            // set the coordinates randomly to either TRUE or FALSE
            unsigned int pos = gol_2dto1d(game, x, y);
            game->board[pos] = (bool) (rand() % 2); // NOLINT(cert-msc50-cpp)
        }
    }
    return GOL_ERR_OK;
}

// PRIVATE
// Calculates the next condition of the cell (alive or dead) based on whether the cell is alive and the number of live neighbors it has
// Returns: The next condition of the cell
static bool gol_cellnext(bool is_alive, int live_neighbors) {
    // condition for this cell to survive if it's alive
    bool survive = is_alive && live_neighbors >= 2 && live_neighbors <= 3;
    // condition for this cell to become alive through reproduction
    bool reproduce = !is_alive && live_neighbors == 3;

    // return whether it is still alive or has reproduced
    // all other conditions means that the cell dies
    return survive || reproduce;
}

// Ticks the progress of the gameoflife board forward, calculating the next state of each cell
// Possible errors (return value): GOL_ERR_NOMEM, GOL_ERR_INIT, GOL_ERR_OK
gol_err gol_tick(struct gameoflife *game) {
    // stores the GOL error of various different API calls
    gol_err error;

    // create a copy of the entire game
    // this is to ensure we can't counting or not counting cells that have been permutated this tick
    struct gameoflife copy;
    if ((error = gol_copy(&copy, game)) != GOL_ERR_OK) return error;

    // loop through each position on the game's board
    for (gol_pos y = 0; y < game->rows; y++) {
        for (gol_pos x = 0; x < game->cols; x++) {
            // find the current value of this cell
            unsigned int pos = gol_2dto1d(game, x, y);
            bool is_alive = game->board[pos];

            // count the number of alive neighbors (out of 8)
            // use the board copy so that we use a clean board
            int live_neighbors;
            if ((error = gol_countlive(&copy, x, y, &live_neighbors)) != GOL_ERR_OK) return error;

            // update the cell value with the new next value
            game->board[pos] = gol_cellnext(is_alive, live_neighbors);
        }
    }

    gol_free(&copy);
    return GOL_ERR_OK;
}

// Converts the gameoflife struct board into a 1d character list, with each row separated by a new line
// This function does allocate the final dest pointer, which must be freed by the user
// For custom ALIVE/DEAD character, provide src with atleast 2 characters, otherwise NULL
// Possible Errors (return value): GOL_ERR_INIT, GOL_ERR_NOMEM, GOL_ERR_OK
gol_err gol_tostring(struct gameoflife *game, char **dest, const char *src) {
    if (game->board == NULL) return GOL_ERR_INIT;

    // allocate memory for a string that contains all positions
    size_t len = ((size_t)game->cols + 1) /* +1 for '\n' */ * game->rows;
    char *str = (char *) malloc(sizeof(char) * (len + 1 /* +1 for '\0' */));
    if (str == NULL) return GOL_ERR_NOMEM;

    // get the characters to use from 'src'
    // if 'src' is NULL, then use defaults instead
    char on_char, off_char;
    if (src == NULL) {
        on_char = 'X';
        off_char = 'O';
    } else {
        on_char = src[0];
        off_char = src[1];
    }

    // insert the characters into the string
    int i = 0;
    for (gol_pos y = 0; y < game->rows; y++) {
        if (y != 0) str[i++] = '\n';
        for (gol_pos x = 0; x < game->cols; x++) {
            // set the character based on the value from the board
            unsigned int pos = gol_2dto1d(game, x, y);
            if (game->board[pos] == true) str[i++] = on_char;
            else str[i++] = off_char;
        }
    }
    // finally null-terminate the string
    str[i] = (char)'\0';

    *dest = str;
    return GOL_ERR_OK;
}
