/*
 * find.c - recursively search for files with a given name in a directory tree
 *
 * Usage: find <directory> <filename> [-exec <cmd> [args...]]
 *
 * Finds all files/directories matching <filename> under <directory>.
 * If -exec is specified, runs <cmd> [args...] <found_file> for each match
 * instead of printing the path.
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

/* Global variables for -exec command */
static int use_exec = 0;
static char **exec_argv;
static int exec_argc;

/*
 * basename - return a pointer to the last component of path (after the last '/').
 */
static char *
basename(char *path)
{
  char *p;
  /* Find first character after last slash */
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

/*
 * run_exec - fork and exec the -exec command with the found file appended
 * as the last argument.
 */
static void
run_exec(char *filepath)
{
  char *argv[MAXARG];
  int i;

  /* Build argv: exec_argv[0..exec_argc-1] followed by filepath */
  for (i = 0; i < exec_argc && i < MAXARG - 2; i++)
    argv[i] = exec_argv[i];
  argv[i] = filepath;
  argv[i + 1] = 0;

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "find: fork failed\n");
    return;
  }
  if (pid == 0) {
    /* Child: execute the command */
    exec(argv[0], argv);
    fprintf(2, "find: exec %s failed\n", argv[0]);
    exit(1);
  }
  /* Parent: wait for child */
  wait(0);
}

/*
 * find - recursively search for files named 'target' under 'path'.
 */
static void
find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_DEVICE:
  case T_FILE:
    /* Check if this file's name matches the target */
    if (strcmp(basename(path), target) == 0) {
      if (use_exec)
        run_exec(path);
      else
        printf("%s\n", path);
    }
    break;

  case T_DIR:
    /* Check if the directory name itself matches */
    if (strcmp(basename(path), target) == 0) {
      if (use_exec)
        run_exec(path);
      else
        printf("%s\n", path);
    }

    /* Make sure the path buffer is large enough */
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "find: path too long\n");
      break;
    }

    /* Build the prefix for entries: path + '/' */
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    /* Iterate over directory entries */
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;

      /* Skip "." and ".." to avoid infinite recursion */
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      /* Build full path for this entry */
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      /* Recurse into the entry */
      find(buf, target);
    }
    break;
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if (argc < 3) {
    fprintf(2, "Usage: find <directory> <filename> [-exec <cmd> [args...]]\n");
    exit(1);
  }

  /* Check for -exec flag */
  for (i = 3; i < argc; i++) {
    if (strcmp(argv[i], "-exec") == 0) {
      if (i + 1 >= argc) {
        fprintf(2, "find: -exec requires a command\n");
        exit(1);
      }
      use_exec = 1;
      exec_argv = &argv[i + 1];
      exec_argc = argc - (i + 1);
      break;
    }
  }

  find(argv[1], argv[2]);

  exit(0);
}
