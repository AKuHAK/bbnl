#include "common.h"
#include "config.h"
#include "loader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Launches the ELF, passing all arguments as-is
// File name must be a relative path to ELF file.
int launchELF(char *fileName, int argc, ELFArgument *args) {
  char *filePath = malloc(sizeof(BDM_MOUNTPOINT) + strlen(fileName));
  strcpy(filePath, BDM_MOUNTPOINT);
  strcat(filePath, fileName);

  if (tryFile(filePath)) {
    printf("ERROR: %s is inaccessible\n", filePath);
    return -ENOENT;
  }

  argc += 1;
  char **argv = malloc(argc * sizeof(char *));
  argv[0] = filePath;

  printf("Launching %s with %d arguments:\n", argv[0], argc-1);
  // Assemble arguments
  ELFArgument *arg = args;
  ELFArgument *narg = NULL;

  for (int i = 1; i < argc; i++) {
    printf("argv[%d]: %s\n", i, arg->arg);
    argv[i] = arg->arg;

    narg = arg->next;
    free(arg);

    if (!narg)
      break;

    arg = narg;
  }

  printf("ERROR: Failed to load %s: %d\n", argv[0], LoadELFFromFile(argc, argv));
  return -ENOENT;
}

