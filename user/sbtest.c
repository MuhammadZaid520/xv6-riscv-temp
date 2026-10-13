/*
 * sbtest.c - test program for sandbox system call
 *
 * Tests various sandbox scenarios: getpid restriction, fork restriction,
 * escape attempts, and path-based restrictions.
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void
report(char *what, int fd)
{
  if (fd < 0) {
    printf("sbtest: forkpath child %s denied\n", what);
  } else {
    printf("sbtest: forkpath child %s ok\n", what);
    close(fd);
  }
}

int
main(int argc, char *argv[])
{
  int result;

  if (argc != 2) {
    fprintf(2, "Usage: sbtest getpid|fork|escape|forkpath\n");
    exit(1);
  }

  if (strcmp(argv[1], "getpid") == 0) {
    printf("sbtest: getpid returned %d\n", getpid());
  } else if (strcmp(argv[1], "fork") == 0) {
    result = fork();
    if (result < 0) {
      printf("sbtest: fork denied\n");
    } else if (result == 0) {
      printf("sbtest: fork child running\n");
      exit(0);
    } else {
      wait(0);
      printf("sbtest: fork ok\n");
    }
  } else if (strcmp(argv[1], "escape") == 0) {
    /* Try to remove sandbox via interpose(0, "-") */
    if (interpose(0, "-") < 0) {
      printf("sbtest: escape interpose failed\n");
    } else {
      printf("sbtest: escape interpose ok\n");
    }
    result = fork();
    if (result < 0) {
      printf("sbtest: escape fork denied\n");
    } else if (result == 0) {
      printf("sbtest: escape fork child running\n");
      exit(0);
    } else {
      wait(0);
      printf("sbtest: escape fork ok\n");
    }
  } else if (strcmp(argv[1], "forkpath") == 0) {
    result = fork();
    if (result < 0) {
      printf("sbtest: forkpath fork denied\n");
    } else if (result == 0) {
      report("README", open("README", O_RDONLY));
      report("Makefile", open("Makefile", O_RDONLY));
      exit(0);
    } else {
      wait(0);
      printf("sbtest: forkpath parent done\n");
    }
  } else {
    fprintf(2, "sbtest: unknown test %s\n", argv[1]);
    exit(1);
  }

  exit(0);
}
