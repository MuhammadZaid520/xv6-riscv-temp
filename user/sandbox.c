/*
 * sandbox.c - sandbox a process by restricting system calls
 *
 * Usage: sandbox <mask> <allowed-path> <command> [args...]
 * Forks a child, calls interpose() with the given mask and path,
 * then execs the specified command in the sandboxed child.
 */
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid;

  if (argc < 4) {
    fprintf(2, "usage: sandbox mask allowed-path command [args...]\n");
    exit(1);
  }

  pid = fork();
  if (pid < 0) {
    fprintf(2, "sandbox: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    if (interpose(atoi(argv[1]), argv[2]) < 0) {
      fprintf(2, "sandbox: interpose failed\n");
      exit(1);
    }
    exec(argv[3], &argv[3]);
    fprintf(2, "sandbox: exec %s failed\n", argv[3]);
    exit(1);
  }

  if (wait(0) < 0) {
    fprintf(2, "sandbox: wait failed\n");
    exit(1);
  }

  exit(0);
}
