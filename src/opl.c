#include "common.h"
#include "config.h"
#include "loader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_OPL_PARTITION "hdd0:+OPL"
#define OPL_CONF_PARTITION_ARG "hdd_partition"

static char oplPath[] = BDM_MOUNTPOINT "/bbnl/OPNPS2LD.elf";

// Launches the ISO via OPL.
// Assumes OPL partition is mounted.
int launchOPL(char *filePath, char *titleID, DiscType mediaType) {
  // Build argv and launch the ELF file
  /* argv[0] ELF boot path
     argv[1] file name (including extension)
     argv[2] title ID
     argv[3] disc type
     argv[4] "bdm" */
  int argCount = 5;
  char **argv = malloc(argCount * sizeof(char *));

  if (tryFile(oplPath)) {
    printf("ERROR: OPNPS2LD.ELF is inaccessible\n");
    return -ENOENT;
  }

  argv[0] = oplPath;
  argv[1] = filePath;
  argv[2] = titleID;
  switch (mediaType) {
  case DISC_TYPE_CD:
    argv[3] = "CD";
    break;
  case DISC_TYPE_DVD:
    argv[3] = "DVD";
    break;
  default:
    printf("ERROR: Unknown disc type\n");
    return -1;
  }
  argv[4] = "bdm";

  printf("Launching %s with arguments:\n", filePath);
  for (int i = 0; i < argCount; i++) {
    printf("%d: %s\n", i + 1, argv[i]);
  }
  printf("ERROR: Failed to load %s: %d\n", argv[0], LoadELFFromFile(argCount, argv));
  return -ENOENT;
}
