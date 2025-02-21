#include <fcntl.h>
#include <ps2sdkapi.h>

// Tests if file exists by opening it
int tryFile(char *filePath) {
  int fd = open(filePath, O_RDONLY);
  if (fd < 0) {
    return fd;
  }
  close(fd);
  return 0;
}
