#ifndef CS540_02_SUDOKU_H
#define CS540_02_SUDOKU_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <pthread.h>

#define SUDOKU_FILTER ", \n\r"    //  For separating out a file
#define SUDOKU_HEIGHT (9)
#define SUDOKU_WIDTH (9)
#define SUDOKU_ARRAY_SIZE (SUDOKU_HEIGHT * SUDOKU_WIDTH) // Total array size
#define MIN_FILE_SIZE (SUDOKU_ARRAY_SIZE * 2 - 1) //  size for numbers times 2 for commas and newlines.

bool get_board(const char *filename, uint32_t **output);
void thread_print(const char *, ...);

#endif
