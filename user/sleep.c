/*
 * sleep.c - user program to pause for a specified number of ticks
 *
 * Usage: sleep <ticks>
 * Pauses execution for the given number of timer ticks.
 * Prints an error message if no argument is provided.
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int ticks;

  if (argc != 2) {
    fprintf(2, "Usage: sleep <ticks>\n");
    exit(1);
  }

  ticks = atoi(argv[1]);
  if (ticks < 0) {
    fprintf(2, "sleep: invalid number of ticks\n");
    exit(1);
  }

  /* Use the pause() system call to sleep for the given ticks */
  if (pause(ticks) < 0) {
    fprintf(2, "sleep: pause failed\n");
    exit(1);
  }

  exit(0);
}
