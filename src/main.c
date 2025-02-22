#include "common.h"
#include "config.h"
#include "elf.h"
#include "game_id.h"
#include "loader.h"
#include "module_init.h"
#include "neutrino.h"
#include "opl.h"
#include "pops.h"
#include <errno.h>
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>

// Waits for HDD modules to initialize
int waitForHDD();

int main(int argc, char *argv[]) {
  printf("*************\nBBN Launcher\n%s\nby pcm720 and CosmicScale\n*************\n", GIT_VERSION);
  int res;

  printf("Loading modules...\n");
  if ((res = initModules()) != 0) {
    printf("ERROR: Failed to initialize modules: %d\n", res);
    return -1;
  }

  // Init devices
  printf("Waiting for HDD\n");
  if (waitForHDD() < 0) {
    printf("ERROR: Failed to initialize HDD\n");
    return -1;
  }

  // Load configuration file.
  // argv[0] should contain the path we can use to determine the APA partition name
  printf("Loading configuration file\n");
  LauncherConfig *config = parseConfig(argv[0]);
  if (!config) {
    printf("ERROR: Failed to load configuration file\n");
    return -1;
  }

  if (config->launcher == LAUNCHER_ELF) {
    launchELF(config->fileName, config->argCount, config->args);
    return -1;
  }

  drawTitleID(config->titleID);
  switch (config->launcher) {
  case LAUNCHER_POPS:
    launchPOPS(config->fileName);
    break;
  case LAUNCHER_OPL:
    launchOPL(config->fileName, config->titleID, config->type);
    break;
  case LAUNCHER_NEUTRINO:
    launchNeutrino(config->fileName, config->type);
    break;
  default:
    printf("ERROR: Unsupported launcher type\n");
  }
  return -1;
}

void delay(int count) {
  int ret;
  for (int i = 0; i < count; i++) {
    ret = 0x01000000;
    while (ret--)
      asm("nop\nnop\nnop\nnop");
  }
}

// Waits for HDD modules to initialize
int waitForHDD() {
  DIR *directory;
  int delayAttempts = 20;

  printf("Trying to open %s\n", BDM_MOUNTPOINT);
  // Wait for IOP to initialize device driver
  for (int attempts = 0; attempts < delayAttempts; attempts++) {
    directory = opendir(BDM_MOUNTPOINT);
    if (directory != NULL) {
      closedir(directory);
      return 0;
    }
    delay(2);
  }

  return -ENODEV;
}
