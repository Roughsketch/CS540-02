#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "sudoku_check.h"

int main(int argc, char *argv[])
{
  bool board_valid = true;
  uint32_t *board = NULL;
  pthread_t check_thread[THREAD_TYPE_COUNT][9];

  //  If an invalid amount of arguments have been passed them show them how to use
  //  the program correctly and then quit.
  if (argc != 2)
  {
    puts("Usage: sudoku <file>");
    return EXIT_SUCCESS;
  }

  //  Get the board. If it retuns false then there was a fatal error.
  if (!get_board(argv[1], &board))
  {
    printf("Board could not be read from '%s' or is invalid\n", argv[1]);
    return EXIT_FAILURE;
  }

  //  For debug purposes, print out the array that we populated to check if it's correct.
  //  Made it print pretty lines because why not.

  puts("+-------+-------+-------+");  //  Top of box
  for (uint32_t row = 0; row < 9; row++)
  {
    for (uint32_t column = 0; column < 9; column++)
    {
      //  If start of column then print left border
      if (!column)
      {
        printf("| ");
      }

      //  Print number then if this column is a multiple of 3 print a border
      printf("%d %s", board[row * SUDOKU_HEIGHT + column], (((column + 1) % 3) == 0 ? "| " : ""));
    }

    printf("\n"); //  Next line

    //  If the row is a multiple of 3 then print a bottome border
    if (((row + 1) % 3) == 0)
    {
      puts("+-------+-------+-------+");
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
    info->box = 0;      //  Set box number
    info->valid = true; //  Assume valid until proven otherwise

    //  Copy the data from the board into the structure's array
    for (uint32_t j = 0; j < 9; j++)
    {
      info->section[j] = board[i * SUDOKU_HEIGHT + j];
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
    info->box = 0;      //  Set box number
    info->valid = true; //  Assume valid until proven otherwise

    //  Copy data into structure's array
    for (uint32_t j = 0; j < 9; j++)
    {
      info->section[j] = board[j * SUDOKU_HEIGHT + i];
    }

    //  Create thread and pass it the structure
    pthread_create(&check_thread[THREAD_COL][i], NULL, check_section, info);
  }

  //  Repeat the same steps as above but this time check each box
  for (uint32_t i = 0; i < 9; i++)
  {
    //  Allocate ThreadInfo structure to store data
    ThreadInfo *info = calloc(1, sizeof(ThreadInfo));
    info->box = i;      //  Set box number
    info->column = 0;   //  Ignore column index
    info->row = 0;      //  Ignore row index
    info->valid = true; //  Assume valid until proven otherwise


    //  Copy data into structure's array
    //  Not pretty at all.
    if (i < 3)
    {
      info->section[0] = board[i * 3];
      info->section[1] = board[i * 3 + 1];
      info->section[2] = board[i * 3 + 2]; 

      info->section[3] = board[i * 3 + 9];
      info->section[4] = board[i * 3 + 1 + 9];
      info->section[5] = board[i * 3 + 2 + 9]; 

      info->section[6] = board[i * 3 + 18];
      info->section[7] = board[i * 3 + 1 + 18];
      info->section[8] = board[i * 3 + 2 + 18]; 
    }
    else if (i < 6)
    {
      info->section[0] = board[(i - 3) * 3 + 27];
      info->section[1] = board[(i - 3) * 3 + 1 + 27];
      info->section[2] = board[(i - 3) * 3 + 2 + 27]; 

      info->section[3] = board[(i - 3) * 3 + 9 + 27];
      info->section[4] = board[(i - 3) * 3 + 1 + 9 + 27];
      info->section[5] = board[(i - 3) * 3 + 2 + 9 + 27]; 

      info->section[6] = board[(i - 3) * 3 + 18 + 27];
      info->section[7] = board[(i - 3) * 3 + 1 + 18 + 27];
      info->section[8] = board[(i - 3) * 3 + 2 + 18 + 27]; 
    }
    else
    {
      info->section[0] = board[(i - 6) * 3 + 54];
      info->section[1] = board[(i - 6) * 3 + 1 + 54];
      info->section[2] = board[(i - 6) * 3 + 2 + 54]; 

      info->section[3] = board[(i - 6) * 3 + 9 + 54];
      info->section[4] = board[(i - 6) * 3 + 1 + 9 + 54];
      info->section[5] = board[(i - 6) * 3 + 2 + 9 + 54]; 

      info->section[6] = board[(i - 6) * 3 + 18 + 54];
      info->section[7] = board[(i - 6) * 3 + 1 + 18 + 54];
      info->section[8] = board[(i - 6) * 3 + 2 + 18 + 54]; 
    }

    //  Create thread and pass it the structure
    pthread_create(&check_thread[THREAD_BOX][i], NULL, check_section, info);
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

  //  Wait for all the threads to come back
  for (int i = 0; i < 9; i++)
  {
    //  Create a variable to hold the incoming return value
    ThreadInfo *return_value = NULL;

    //  Wait for the thread and get the return value
    pthread_join(check_thread[THREAD_BOX][i], (void *)&return_value);

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