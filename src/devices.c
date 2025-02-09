#include "devices.h"
#include <hdd-ioctl.h>
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <io_common.h>

void delay(int count);

static char *mountpoints[] = {
    "hdd0:", // APA
    "mass0:" // exFAT partition
};

// Waits for HDD modules to initialize
int initDevices() {
  DIR *directory;
  int delayAttempts = 20;

  for (int i = 0; i < sizeof(mountpoints) / sizeof(char *); i++) {
    printf("Trying to open %s\n", mountpoints[i]);
    // Wait for IOP to initialize device driver
    for (int attempts = 0; attempts < delayAttempts; attempts++) {
      directory = opendir(mountpoints[i]);
      if (directory != NULL) {
        closedir(directory);
        break;
      }
      delay(5);
    }
    if (directory == NULL) {
      // All mountpoints must exist
      return -ENODEV;
    }
  }
  return 0;
}

// Attempts to mount PFS partition read-only
int mountPFS(char *partitionPath) {
  char mountpoint[] = "pfs0:";

  printf("Mounting %s as pfs0:\n", partitionPath);
  if (fileXioMount(mountpoint, partitionPath, FIO_MT_RDONLY)) {
    free(partitionPath);
    return -ENODEV;
  }

  printf("Mounted %s as pfs0:\n", partitionPath);
  return 0;
}

// Unmounts PFS partition mounted at pfs0:
void unmountPFS() {
  fileXioDevctl("pfs:", PDIOC_CLOSEALL, NULL, 0, NULL, 0);
  fileXioSync("pfs0:", FXIO_WAIT);
  fileXioUmount("pfs0:");
}

void delay(int count) {
  int ret;
  for (int i = 0; i < count; i++) {
    ret = 0x01000000;
    while (ret--)
      asm("nop\nnop\nnop\nnop");
  }
}
