/*
 * memdump.c - prints memory contents according to a format string
 *
 * Format characters:
 *   i: print next 4 bytes as a 32-bit integer (decimal)
 *   p: print next 8 bytes as a 64-bit integer (hex)
 *   h: print next 2 bytes as a 16-bit integer (decimal)
 *   c: print next 1 byte as an 8-bit ASCII character
 *   s: next 8 bytes are a 64-bit pointer to a C string; print the string
 *   S: rest of data is a null-terminated string; print it
 *
 * memdump() respects the len argument and never reads past valid data.
 */
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else {
    /* Read stdin and use argv[1] as the format string */
    char buf[4096];
    int n;
    int total = 0;

    while ((n = read(0, buf + total, sizeof(buf) - total)) > 0)
      total += n;

    memdump(argv[1], buf, total);
  }

  exit(0);
}

/*
 * memdump - prints memory contents according to a format string.
 *
 * fmt:  format string with characters i, p, h, c, s, S
 * data: pointer to the memory region to dump
 * len:  number of valid bytes in data
 */
void
memdump(char *fmt, char *data, int len)
{
  int offset = 0; /* current byte position within data */

  for (int i = 0; fmt[i] != '\0'; i++) {
    char f = fmt[i];

    switch (f) {
    case 'i': {
      /* Print 4 bytes as a 32-bit signed integer in decimal */
      if (offset + 4 > len) {
        printf("memdump: not enough data for 'i'\n");
        return;
      }
      int val;
      memmove(&val, data + offset, sizeof(int));
      printf("%d\n", val);
      offset += 4;
      break;
    }

    case 'p': {
      /* Print 8 bytes as a 64-bit integer in hex (no 0x prefix) */
      if (offset + 8 > len) {
        printf("memdump: not enough data for 'p'\n");
        return;
      }
      uint64 val;
      memmove(&val, data + offset, sizeof(uint64));
      /* Print as hex without 0x prefix, suppressing leading zeros */
      if (val == 0) {
        printf("0\n");
      } else {
        char hexbuf[17];
        char hexdigits[] = "0123456789ABCDEF";
        int pos = 16;
        hexbuf[pos] = '\0';
        uint64 v = val;
        while (v > 0) {
          pos--;
          hexbuf[pos] = hexdigits[v & 0xf];
          v >>= 4;
        }
        printf("%s\n", hexbuf + pos);
      }
      offset += 8;
      break;
    }

    case 'h': {
      /* Print 2 bytes as a 16-bit integer in decimal */
      if (offset + 2 > len) {
        printf("memdump: not enough data for 'h'\n");
        return;
      }
      short val;
      memmove(&val, data + offset, sizeof(short));
      printf("%d\n", val);
      offset += 2;
      break;
    }

    case 'c': {
      /* Print 1 byte as an ASCII character */
      if (offset + 1 > len) {
        printf("memdump: not enough data for 'c'\n");
        return;
      }
      char val = data[offset];
      printf("%c\n", val);
      offset += 1;
      break;
    }

    case 's': {
      /* Next 8 bytes contain a pointer to a C string; print the string */
      if (offset + 8 > len) {
        printf("memdump: not enough data for 's'\n");
        return;
      }
      char *ptr;
      memmove(&ptr, data + offset, sizeof(char *));
      printf("%s\n", ptr);
      offset += 8;
      break;
    }

    case 'S': {
      /* Rest of data is a null-terminated string; print it */
      int remaining = len - offset;
      if (remaining <= 0) {
        printf("memdump: not enough data for 'S'\n");
        return;
      }
      /* Print characters until null or end of valid data */
      int j;
      for (j = 0; j < remaining; j++) {
        char ch = data[offset + j];
        if (ch == '\0')
          break;
        printf("%c", ch);
      }
      printf("\n");
      offset = len; /* S consumes remaining data */
      break;
    }

    default:
      /* Unknown format character, skip */
      break;
    }
  }
}
