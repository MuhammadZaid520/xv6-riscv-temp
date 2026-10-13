/*
 * sixfive.c - prints all numbers from input that are multiples of 5 or 6
 *
 * Numbers are sequences of decimal digits separated by characters in
 * the string " -\r\t\n./,". Start and end of file are implicit separators.
 * If no command-line arguments, reads from stdin.
 * If a file cannot be opened, prints "sixfive: cannot open <filename>".
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/* Separator characters that delimit numbers */
static char separators[] = " -\r\t\n./,";

/*
 * process_fd - reads from a file descriptor and prints numbers
 * that are multiples of 5 or 6.
 */
static void
process_fd(int fd)
{
  char c;
  int n;
  int num = 0;
  int have_digits = 0;
  int overflow = 0;

  while ((n = read(fd, &c, 1)) > 0) {
    if (strchr(separators, c) != 0) {
      /* Separator found: check and possibly print the accumulated number */
      if (have_digits && !overflow && (num % 5 == 0 || num % 6 == 0))
        printf("%d\n", num);
      /* Reset state for next number */
      num = 0;
      have_digits = 0;
      overflow = 0;
    } else if (c >= '0' && c <= '9') {
      /* Digit: accumulate into the current number */
      int digit = c - '0';
      /* Check for integer overflow */
      if (num > (0x7fffffff - digit) / 10) {
        overflow = 1;
      }
      num = num * 10 + digit;
      have_digits = 1;
    } else {
      /*
       * Non-separator, non-digit character (e.g., a letter).
       * This invalidates the current token, so discard accumulated digits.
       * Characters like letters in "xv6" mean the '6' is NOT a standalone number.
       */
      have_digits = 0;
      num = 0;
      overflow = 0;
      /* Skip until the next separator */
      while ((n = read(fd, &c, 1)) > 0) {
        if (strchr(separators, c) != 0)
          break;
      }
      /* After hitting a separator (or EOF), reset for next token */
      num = 0;
      have_digits = 0;
      overflow = 0;
    }
  }

  /* End of file acts as an implicit separator */
  if (have_digits && !overflow && (num % 5 == 0 || num % 6 == 0))
    printf("%d\n", num);
}

int
main(int argc, char *argv[])
{
  int fd;
  int i;

  if (argc <= 1) {
    /* No arguments: read from stdin */
    process_fd(0);
  } else {
    for (i = 1; i < argc; i++) {
      fd = open(argv[i], O_RDONLY);
      if (fd < 0) {
        fprintf(2, "sixfive: cannot open %s\n", argv[i]);
        continue;
      }
      process_fd(fd);
      close(fd);
    }
  }

  exit(0);
}
