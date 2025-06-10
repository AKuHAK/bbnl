#include "common.h"
#include "loader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char popstarterPath[] = PFS_MOUNTPOINT "/bbnl/POPSTARTER.ELF";

// Launches POPS
int launchPOPS(char *fileName) {
  // Remove extension from the file name
  char *ext = strrchr(fileName, '.'); // Find the last occurrence of '.'
  if (ext != NULL)
    *ext = '\0';

  if (tryFile(popstarterPath)) {
    printf("ERROR: POPSTARTER.ELF is inaccessible\n");
    return -ENOENT;
  }

  // POPStarter needs ELF path as argv[0].
  // Mountpoint can be anything as long as the path ends with <image name>.ELF,
  // so we'll pass "bbnl:<image name>.ELF" as argv[1].
  //
  // POPStarter will receive this as argv[0] because the ELF loader will
  // replace argv[0] with argv[1] if argv[1] starts with "bbnl"
  char **argv = malloc(2 * sizeof(char *));
  int argSize = strlen(fileName) + 10;
  argv[0] = popstarterPath;
  argv[1] = calloc(sizeof(char), argSize);
  snprintf(argv[1], argSize, "bbnl:%s.ELF", fileName);

  printf("Launching %s with %s\n", argv[0], argv[1]);
  printf("ERROR: Failed to load %s: %d\n", argv[0], LoadELFFromFile(2, argv));
  return -ENOENT;
}
