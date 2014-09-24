#ifndef CS540_02_SUDOKU_CHECK_H
#define CS540_02_SUDOKU_CHECK_H

#include "sudoku.h"

#define THREAD_ROW 0
#define THREAD_COL 1
#define THREAD_BOX 2
#define THREAD_TYPE_COUNT 3

typedef struct board_info
{
  uint32_t row;
  uint32_t column;
  uint32_t section[9];
  bool valid;
} ThreadInfo;

void *check_section(void *);
#endif
