#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#define SUDOKU_FILTER ", \n\r"
#define MIN_FILE_SIZE (9 * 9 * 2) // 9 * 9 for numbers, times 2 for commas and newlines.

int main(int argc, char *argv[])
{
  int board[9][9];
  char *input_str = NULL;
  pthread_t check_thread[9];

  size_t file_size = 0;
  //  Open the input file in binary read mode so return carriages aren't skipped
  FILE *input = fopen(argv[1], "rb");

  //  If there was some error then tell the user and quit
  if (input == NULL)
  {
    printf("Could not open file: %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  //  Get the file size
  fseek(input, 0, SEEK_END);
  file_size = ftell(input);
  rewind(input);

  //  If the file size is less than the lowest possible byte count for a board then tell the user.
  if (file_size < MIN_FILE_SIZE)
  {
    printf("File is too small to hold a complete sudoku board: %d %d\n", MIN_FILE_SIZE, file_size);
    return EXIT_FAILURE;
  }

  //  Allocate enough space to store the entire file and read it in
  input_str = calloc(file_size + 1, sizeof(char));
  if (fread(input_str, sizeof(char), file_size, input) != file_size)
  {
    puts("Could not read all data from the file.");
    return EXIT_FAILURE;
  }

  //  Split the file's contents by commas, spaces, and line separators and grab each number
  int row = 0, column = 0;
  char *tokptr = strtok(input_str, SUDOKU_FILTER);

  //  While we have a next number and are within the boards for the board array
  while (tokptr != NULL && row < 9)
  {
    //  Print out the value read and the coordinates in the board
    printf("Found '%s' at (%d, %d)\n", tokptr, row, column);

    int entry = strtol(tokptr, NULL, 10);

    //  If entry is not valid for a sudoku board then show the error here
    if (entry == 0 || entry > 9)
    {
      printf("Board is invalid: Invalid number at position (%d, %d).\n", row + 1, column + 1);
      return EXIT_SUCCESS;
    }

    //  Put the number into the array
    board[row][column] = entry;

    //  Get next number
    tokptr = strtok(NULL, SUDOKU_FILTER);

    if (column + 1 == 9)
    {
      row++;
    }

    column = (column + 1) % 9;
  }

  //  Free memory because that's what you're supposed to do.
  free(input_str);

  //  If row is not 9, then we undershot the array
  if (row != 9)
  {
    puts("Invalid array length. Check that the file is a full sudoku board.");
    return EXIT_FAILURE;
  }

  //  For debug purposes, print out the array that we populated to check if it's correct.
  for (row = 0; row < 9; row++)
  {
    for (column = 0; column < 9; column++)
    {
      printf("%d ", board[row][column]);
    }
    printf("\n");
  }

  return EXIT_SUCCESS;
}