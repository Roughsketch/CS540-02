/*
  TODO:
    Move definitions to a header file
    Create functions for bulk of main code to organize things
    Add sequence check for each quadrant
    Change thread joining to not be deterministic
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define SUDOKU_FILTER ", \n\r"
#define MIN_FILE_SIZE (9 * 9 * 2) //  9 * 9 for numbers, times 2 for commas and newlines.

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

/*
  Summary: Checks a section of a sudoku board (line, column, or box)

  Parameters:
    void *data: A void pointer that can be cast into a ThreadInfo structure
*/
void *check_section(void *data)
{
  //  Get the pointer of the ThreadInfo structure
  ThreadInfo *info = (ThreadInfo *)(data);

  //  Set up start state
  bool seen[9] = { false }; //  Seen test (one of each value allowed per set)
  char output[512];         //  Used to printf entire lines while in a thread

  //  Go through each number in the set
  for (uint32_t i = 0; i < 9; i++)
  {
    //  Store current number to avoid indexing time
    int current = info->section[i];

    if (seen[current] == false)
    {
      seen[current] = true; 
    }
    else
    {
      sprintf(output, "[Error]:\t%s %d contains more than one '%d' entry at (%d, %d).",
                info->row ? "Row" : "Column",                 //  If prints whether we're checking row or column
                (info->row ? info->row : info->column) + 1,   //  Print row or column number
                current,                                      //  Print current value
                (info->row ? info->row : i) + 1,              //  If we're using rows use value, otherwise use iterator
                (info->column ? info->column : i) + 1);       //  Same as above for column

      puts(output);

      //  Set invalid
      info->valid = false;
    }
  }
  
  //  Return the pointer to the data that was altered in this thread
  //  The structure is allocated dynamically so it does not go out of scope
  pthread_exit(data);

  return NULL;  //  Useless return to avoid warnings on compilation
}

int main(int argc, char *argv[])
{
  uint32_t board[9][9];
  bool board_valid = true;
  char *input_str = NULL;
  pthread_t check_thread[THREAD_TYPE_COUNT][9];

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
  uint32_t row = 0, column = 0;
  char *tokptr = strtok(input_str, SUDOKU_FILTER);

  //  While we have a next number and are within the boards for the board array
  while (tokptr != NULL && row < 9)
  {
    uint32_t entry = strtol(tokptr, NULL, 10);

    //  If entry is not valid for a sudoku board then show the error here
    if (entry == 0 || entry > 9)
    {
      printf("Board is invalid: Invalid number '%d' at position (%d, %d).\n", entry, row + 1, column + 1);
      return EXIT_SUCCESS;
    }

    //  Put the number into the array
    board[row][column] = entry;

    //  Get next number
    tokptr = strtok(NULL, SUDOKU_FILTER);

    //  If column has reached the end, then we're on a new row
    if (column + 1 == 9)
    {
      row++;
    }

    //  Increase column, but if we moved to a new row then set to 0
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
  //  Made it print pretty lines because why not.

  puts("-------------------------");  //  Top of box
  for (row = 0; row < 9; row++)
  {
    for (column = 0; column < 9; column++)
    {
      //  If start of column then print left border
      if (!column)
      {
        printf("| ");
      }

      //  Print number then if this column is a multiple of 3 print a border
      printf("%d %s", board[row][column], (((column + 1) % 3) == 0 ? "| " : ""));
    }

    printf("\n"); //  Next line

    //  If the row is a multiple of 3 then print a bottome border
    if (((row + 1) % 3) == 0)
    {
      puts("-------------------------");
    }
  }

  //  Print two newlines to separate the display from the output.
  puts("\n");

  //  Create 9 threads to evaluate each row of the board
  for (uint32_t i = 0; i < 9; i++)
  {
    //  Allocate a ThreadInfo object and set starting data
    ThreadInfo *info = calloc(1, sizeof(ThreadInfo));
    info->row = i;      //  We are checking rows, so set row
    info->column = 0;   //  Since we are checking rows, set column to 0
    info->valid = true; //  Assume valid until proven otherwise

    //  Copy the data from the board into the structure's array
    for (uint32_t j = 0; j < 9; j++)
    {
      info->section[j] = board[i][j];
    }

    //  Create a thread and pass the pointer to the structure as a parameter
    pthread_create(&check_thread[THREAD_ROW][i], NULL, check_section, info);
  }

  //  Repeat the same steps as above but this time check columns instead of rows
  for (uint32_t i = 0; i < 9; i++)
  {
    //  Allocate ThreadInfo structure to store data
    ThreadInfo *info = calloc(1, sizeof(ThreadInfo));
    info->column = i;   //  We are checking columns this time so set the column index
    info->row = 0;      //  Ignore row index
    info->valid = true; //  Assume valid until proven otherwise

    //  Copy data into structure's array
    for (uint32_t j = 0; j < 9; j++)
    {
      info->section[j] = board[j][i];
    }

    //  Create thread and pass it the structure
    pthread_create(&check_thread[THREAD_COL][i], NULL, check_section, info);
  }

  //  Repeat the same steps as above but this time check each box
  for (uint32_t i = 0; i < 9; i++)
  {
    //  Allocate ThreadInfo structure to store data
    ThreadInfo *info = calloc(1, sizeof(ThreadInfo));
    info->column = i;   //  We are checking columns this time so set the column index
    info->row = 0;      //  Ignore row index
    info->valid = true; //  Assume valid until proven otherwise

    //  Copy data into structure's array
    for (uint32_t j = 0; j < 9; j++)
    {
      info->section[j] = board[j][i];
    }

    //  Create thread and pass it the structure
    pthread_create(&check_thread[THREAD_COL][i], NULL, check_section, info);
  }

  //  Done populating rows, time to draw them back in
  for (int i = 0; i < 9; i++)
  {
    //  Make a variable to store the incoming return value
    ThreadInfo *return_value = NULL;

    //  Wait for each thread to finish and return a structure with the modified data
    pthread_join(check_thread[THREAD_ROW][i], (void *)&return_value);

    //  If this row was invalidated then set the global valid boolean to false
    //  We will keep parsing regardless because it's better to give all errors at once.
    if (return_value->valid == false)
    {
      board_valid = false;
    }

    //  Free the allocated memory because it's a nice thing to do
    free(return_value);
  }

  //  Wait for all the threads to come back
  for (int i = 0; i < 9; i++)
  {
    //  Create a variable to hold the incoming return value
    ThreadInfo *return_value = NULL;

    //  Wait for the thread and get the return value
    pthread_join(check_thread[THREAD_COL][i], (void *)&return_value);

    //  If the structure was marked as invalid then set the global validation to false
    if (return_value->valid == false)
    {
      board_valid = false;
    }

    //  Free memory because if you don't people will swear at you in 10 years.
    free(return_value);
  }


  //  If board_valid is true, then all three cases were validated and thus the board is valid
  if (board_valid)
  {
    puts("This board is valid.");
  }
  else
  {
    //  If not valid then it's not valid. What did you expect?
    puts("This board is invalid");
  }
  return EXIT_SUCCESS;
}