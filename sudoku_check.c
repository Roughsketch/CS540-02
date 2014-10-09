#include "sudoku_check.h"

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
  bool seen[9] = { false };     //  Seen test (one of each value allowed per set)
  bool dup_box[9] = { false };  //  Flags to avoid printing out duplicate errors for boxes

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
      if (info->box)
      {
        if (dup_box[current] == false)
        {
          dup_box[current] = true;
          thread_print("[Error]:\tBox %d contains more than one %d.", info->box, current);
        }
      }
      else
      {
        thread_print("[Error]:\t%s %d contains more than one '%d' entry at (%d, %d).",
                info->row ? "Row" : "Column",                  //  If prints whether we're checking row or column
                (info->row ? info->row : info->column) + 1,   //  Print row or column number
                current,                                      //  Print current value
                (info->row ? info->row : i) + 1,              //  If we're using rows use value, otherwise use iterator
                (info->column ? info->column : i) + 1);       //  Same as above for column 
      }

      //  Set invalid
      info->valid = false;
    }
  }
  
  //  Return the pointer to the data that was altered in this thread
  //  The structure is allocated dynamically so it does not go out of scope
  pthread_exit(data);

  return NULL;  //  Useless return to avoid warnings on compilation
}