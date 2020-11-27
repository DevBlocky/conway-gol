#ifndef C_PLAYGROUND_GOL_H
#define C_PLAYGROUND_GOL_H

#include <stdbool.h>

typedef unsigned int gol_err;
#define GOL_ERR_OK 0
#define GOL_ERR_INIT 1
#define GOL_ERR_NOMEM 2

typedef short int gol_pos;
struct gameoflife {
    gol_pos rows, cols;
    bool *board;
};

// initializes the gameoflife struct with the provided values. allocated the board.
gol_err gol_init(struct gameoflife *game, gol_pos rows, gol_pos cols);
// destructs the game and releases all associated memory (except for the provided pointer itself)
gol_err gol_free(struct gameoflife *game);
// populates the board with random values (doesn't specify srand)
gol_err gol_populate(struct gameoflife *game);
// ticks the board forward, checking all associated rules and making changes accordingly
gol_err gol_tick(struct gameoflife *game);
// places an allocated string into 'dest' of the board, rows separated by newlines
// you must call free after you are done using the value in dest
// src is a char array with a length of 2 providing the on/off values (can be NULL)
gol_err gol_tostring(struct gameoflife *game, char **dest, const char *src);

#endif //C_PLAYGROUND_GOL_H
