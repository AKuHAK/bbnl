#include "devices.h"
#include "loader.h"
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Launches POPS
int launchPOPS(char *filePath) {
  // Remove extension from the file name
  char *ext = strrchr(filePath, '.'); // Find the last occurrence of '.'
  if (ext != NULL)
    *ext = '\0';

  // Mount the __.POPS partition
  if (mountPFS("hdd0:__.POPS")) {
    printf("ERROR: Failed to mount POPS partition\n");
    return -ENODEV;
  }

  // Build argv and launch the ELF file
  char **argv = malloc(sizeof(char *));
  int argSize = strlen(filePath) + 11;
  argv[0] = calloc(sizeof(char), argSize);
  snprintf(argv[0], argSize, "pfs0:/%s.ELF", filePath);

  printf("Launching %s\n", argv[0]);
  printf("ERROR: Failed to load %s: %d\n", argv[0], LoadELFFromFile(1, argv));
  unmountPFS();
  return -ENOENT;
}
