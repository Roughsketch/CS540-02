#include "sudoku.h"

bool get_board(const char *filename, uint32_t **output)
{
  char *input_str = NULL;
  size_t file_size = 0;
  //  Open the input file in binary read mode so return carriages aren't skipped
  FILE *input = fopen(filename, "rb");

  //  If there was some error then tell the user and quit
  if (input == NULL)
  {
    printf("Could not open file: %s\n", filename);
    return false;
  }

  //  Get the file size
  fseek(input, 0, SEEK_END);
  file_size = ftell(input);
  rewind(input);

  //  If the file size is less than the lowest possible byte count for a board then tell the user.
  if (file_size < MIN_FILE_SIZE)
  {
    printf("File is too small to hold a complete sudoku board: %d %d\n", MIN_FILE_SIZE, file_size);
    return false;
  }

  //  Allocate enough space to store the entire file and read it in
  input_str = calloc(file_size + 1, sizeof(char));
  if (fread(input_str, sizeof(char), file_size, input) != file_size)
  {
    puts("Could not read all data from the file.");
    return false;
  }

  //  Successful so far, allocate space for array
  *output = calloc(SUDOKU_ARRAY_SIZE, sizeof(uint32_t));

  //  Split the file's contents by commas, spaces, and line separators and grab each number
  uint32_t row = 0, column = 0;
  char *tokptr = strtok(input_str, SUDOKU_FILTER);

  //  While we have a next number and are within the boards for the board array
  while (tokptr != NULL && row < SUDOKU_HEIGHT)
  {
    uint32_t entry = strtol(tokptr, NULL, 10);

    //  If entry is not valid for a sudoku board then show the error here
    if (entry == 0 || entry > SUDOKU_WIDTH)
    {
      printf("Invalid number '%d' at position (%d, %d).\n", entry, row + 1, column + 1);
    }

    //  Put the number into the array
    (*output)[(row * SUDOKU_HEIGHT) + column] = entry;

    //  Get next number
    tokptr = strtok(NULL, SUDOKU_FILTER);

    //  If column has reached the end, then we're on a new row
    if (column + 1 == SUDOKU_WIDTH)
    {
      row++;
    }

    //  Increase column, but if we moved to a new row then set to 0
    column = (column + 1) % SUDOKU_WIDTH;
  }

  //  Free memory because that's what you're supposed to do.
  free(input_str);

  //  If row is not 9, then we undershot the array
  if (row != SUDOKU_HEIGHT)
  {
    puts("Invalid array length. Check that the file is a full sudoku board.");
    return false;
  }

  return true;
}