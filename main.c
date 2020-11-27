#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "gol.h"

#define ROWS 30
#define COLS 120

// random seed generator for different systems
#ifdef WIN32

#include <Windows.h>

#define seed_rand srand(GetTickCount())
#else
#include <sys/time.h>
#define seed_rand {\
    struct timeval _time;\
    gettimeofday(&_time, NULL);\
    srand((_time.tv_sec * 1000) + (time.tv_usec / 1000));\
}
#endif

#ifdef WIN32
#define CLEAR_CONSOLE system("cls")
#else
#define CLEAR_CONSOLE system("clear")
#endif

// a non-implementation defined function to wait for the number of milliseconds provided for the calling thread
static int wait_ms(int ms) {
#ifdef WIN32
    Sleep(ms);
    return 0;
#else
    struct timespec time = {
            .tv_sec = ms / 1000,
            .tv_nsec = (ms % 1000) * 1000000
    };
    return nanosleep(&time, NULL);
#endif
}

int main(void) {
    // generate a reliable random seed using the definition
    seed_rand;

    // setup the initial game
    struct gameoflife game;
    if (gol_init(&game, ROWS, COLS) != GOL_ERR_OK) return EXIT_FAILURE;

    // populate the grid randomly
    if (gol_populate(&game) != GOL_ERR_OK) return EXIT_FAILURE;

    char *game_str;
    while (TRUE) {
        // print the board as a string
        if (gol_tostring(&game, &game_str, "X ") != GOL_ERR_OK) return EXIT_FAILURE;
        printf("%s", game_str);
        free(game_str);

        // if the wait is unsuccessful, then break
        if (wait_ms(100) != 0)
            break;

        gol_tick(&game);
        CLEAR_CONSOLE;
    }

    gol_free(&game);

    return EXIT_SUCCESS;
}
