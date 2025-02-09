#include "devices.h"
#include "config.h"
#include "loader.h"
#include <fcntl.h>
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_OPL_PARTITION "hdd0:+OPL"
#define OPL_CONF_PARTITION_ARG "hdd_partition"

char *oplPaths[] = {"pfs0:/OPNPS2LD.ELF", "pfs0:/OPL/OPNPS2LD.ELF"};

// Tests if file exists by opening it
int tryFile(char *filepath) {
  int fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    return fd;
  }
  close(fd);
  return 0;
}

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

  argv[0] = NULL;
  for (int i = 0; i < sizeof(oplPaths) / sizeof(char *); i++) {
    if (!tryFile(oplPaths[i])) {
      argv[0] = oplPaths[i];
      break;
    }
  }
  if (!argv[0]) {
    printf("ERROR: OPNPS2LD.ELF is inaccessible\n");
    return -ENOENT;
  }

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

// Parses OPL configuraton file for OPL partition
int readOPLConfig(char *partitionName) {
  char buf[PATH_MAX];
  buf[0] = '\0';

  FILE *fd = fopen("pfs0:OPL/conf_hdd.cfg", "rb");
  if (!fd)
    return -ENOENT;

  while (fgets(buf, sizeof(buf), fd) != NULL) {
    if (!strncmp(buf, OPL_CONF_PARTITION_ARG, sizeof(OPL_CONF_PARTITION_ARG)))
      continue;

    break;
  }

  char *val = strchr(buf, '=');
  if (!val) {
    return -1;
  }
  val++; // Point to argument value

  // Remove newline from value
  char *newline = NULL;
  if ((newline = strchr(val, '\r')) != NULL)
    *newline = '\0';
  if ((newline = strchr(val, '\n')) != NULL)
    *newline = '\0';

  // Set partition name
  strcpy(partitionName, "hdd0:");
  strcat(partitionName, val);

  // Close the file after reading
  fclose(fd);
  return 0;
}

// Mounts OPL partition
int mountOPLPartition() {
  char partitionName[PATH_MAX];
  partitionName[0] = '\0';

  printf("Reading OPL configuration file from __common\n");
  if (!mountPFS("hdd0:__common")) {
    readOPLConfig(partitionName);
    unmountPFS();
  }

  if (partitionName[0] == '\0')
    strcpy(partitionName, DEFAULT_OPL_PARTITION);

  if (mountPFS(partitionName))
    return -ENODEV;

  return 0;
}
