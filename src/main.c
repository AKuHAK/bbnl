#include "config.h"
#include "devices.h"
#include "game_id.h"
#include "loader.h"
#include "module_init.h"
#include "neutrino.h"
#include "opl.h"
#include "pops.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("*************\nBBN Launcher\n%s\nby pcm720 and CosmicScale\n*************\n", GIT_VERSION);
  int res;

  printf("Loading modules...\n");
  if ((res = initModules()) != 0) {
    printf("ERROR: Failed to initialize modules: %d\n", res);
    return -1;
  }

  // Init devices
  printf("Waiting for HDD mountpoints\n");
  if (initDevices() < 0) {
    printf("ERROR: Failed to initialize devices\n");
    return -1;
  }

  // Load configuration file
  printf("Loading configuration file\n");
  LauncherConfig *config = parseConfig();
  if (!config) {
    printf("ERROR: Failed to parse configuration file\n");
    return -1;
  }

  if (config->launcher == LAUNCHER_POPS) {
    // Launch POPS
    drawTitleID(config->titleID);
    if (launchPOPS(config->fileName)) {
      return -1;
    }
  }

  printf("Mounting OPL partition\n");
  if (mountOPLPartition()) {
    printf("ERROR: Failed to mount OPL partition");
    return -1;
  }

  drawTitleID(config->titleID);
  switch (config->launcher) {
  case LAUNCHER_OPL:
    launchOPL(config->fileName, config->titleID, config->type);
    break;
  case LAUNCHER_NEUTRINO:
    launchNeutrino(config->fileName, config->type);
    break;
  default:
    printf("ERROR: Unknown launcher type\n");
  }

  unmountPFS();
  return -1;
}
