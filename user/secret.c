/*
 * secret.c - writes a secret string to memory and then exits
 *
 * This program allocates memory with sbrk, copies the secret argument
 * into it, and then exits. The memory pages are freed but the data
 * remains (due to the LAB_SYSCALL bug where pages aren't zeroed).
 */
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char *memory;

  if (argc != 2) {
    fprintf(2, "usage: secret value\n");
    exit(1);
  }
  memory = sbrk(strlen(argv[1]) + 1);
  if (memory == SBRK_ERROR) {
    fprintf(2, "secret: sbrk failed\n");
    exit(1);
  }
  strcpy(memory, argv[1]);
  exit(0);
}
