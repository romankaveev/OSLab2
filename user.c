#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

#define BUFFER_SIZE 128

int main(int argc, char *argv[]) {
  int pid;
  bool is_pid = false;
  if (sscanf(argv[1], "--pid=%d", &pid)) is_pid = true;
  FILE *file = fopen("/sys/kernel/debug/laba2/result", "r+");
  if (file == NULL) {
    printf("Error, can not open file\n");
    return 0;
  }
  if (is_pid) {
    char *buffer[BUFFER_SIZE];
    char path[32];
    sprintf(path, "/proc/%d/fd", pid);
    DIR* dir = opendir(path);
    struct dirent *dp = readdir(dir);
    if (dp == NULL) {
      printf("This process does not have any associated fds.");
      fprintf(file, "pid: %d, fd: -1", pid);
    }
    else {
      int fd = dirfd(dp);
      fprintf(file, "pid: %d, fd: %d", pid, fd);
    }
    fprintf(file, "pid: %d", pid);
    while (true) {
      char *result = fgets(buffer, BUFFER_SIZE, file);
      if (feof(file)) break;
      printf(result);
    }
  } else {
      printf("Error with arguments");
  }
  fclose(file);
  return 0;
}
