#include "config.h"
#include "devices.h"
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parses line into LauncherConfig
void parseLine(LauncherConfig *info, char *line) {
  // Find argument name
  char *val = strchr(line, '=');
  if (!val) {
    printf("WARN: Value delimiter not found\n");
    return;
  }

  // Terminate argument and advance pointer to point to value
  *val = '\0';
  val++;

  // Remove newline from value
  char *newline = NULL;
  if ((newline = strchr(val, '\r')) != NULL)
    *newline = '\0';
  if ((newline = strchr(val, '\n')) != NULL)
    *newline = '\0';

  // Parse argument and value
  if (!strcmp(line, "file_name")) {
    info->fileName = strdup(val);
    return;
  }
  if (!strcmp(line, "title_id")) {
    info->titleID = strdup(val);
    return;
  }
  if (!strcmp(line, "disc_type")) {
    if (!strcmp(val, "DVD")) {
      info->type = DISC_TYPE_DVD;
    } else if (!strcmp(val, "CD")) {
      info->type = DISC_TYPE_CD;
    } else if (!strcmp(val, "POPS")) {
      info->type = DISC_TYPE_POPS;
      info->launcher = LAUNCHER_POPS;
    }
    return;
  }
  if (!strcmp(line, "launcher")) {
    if (!strcmp(val, "NEUTRINO")) {
      info->launcher = LAUNCHER_NEUTRINO;
    } else if (!strcmp(val, "POPS")) {
      info->launcher = LAUNCHER_POPS;
    } else { // Use OPL as default launcher
      info->launcher = LAUNCHER_OPL;
    }
    return;
  }
  printf("WARN: Unsupported argument %s\n", line);
}

// Releases memory used by config, including the passed pointer
void freeConfig(LauncherConfig *config) {
  if (!config->fileName)
    free(config->fileName);

  if (!config->titleID)
    free(config->titleID);

  free(config);
}

// Parses configuration file in the current working directory into LauncherConfig
// Returns NULL on failure
LauncherConfig *parseConfig() {
  char buf[PATH_MAX];
  buf[0] = '\0';

  // Get the current working directory
  if (getcwd(buf, PATH_MAX) == NULL) {
    printf("ERROR: Failed to get CWD\n");
    return NULL;
  }

  // Remove ":pfs:" postfix from the current working directory path
  char *pfsPostfix = strstr(buf, ":pfs:");
  if (pfsPostfix)
    *pfsPostfix = '\0';

  // Mount the current working directory
  if (mountPFS(buf)) {
    printf("ERROR: Failed to mount the partition\n");
    return NULL;
  }

  // Open the configuration file
  FILE *fd = fopen("pfs0:launcher.cfg", "rb");
  if (!fd) {
    printf("ERROR: Failed to open launcher.cfg\n");
    unmountPFS();
    return NULL;
  }

  // Reinitialize buffer and initialize config
  memset(&buf, 0, PATH_MAX);
  LauncherConfig *lConfig = malloc(sizeof(LauncherConfig));

  // Set defaults
  lConfig->fileName = NULL;
  lConfig->titleID = NULL;
  lConfig->type = DISC_TYPE_NONE;
  lConfig->launcher = LAUNCHER_OPL;

  // Parse file
  while (fgets(buf, sizeof(buf), fd) != NULL) {
    printf("Parsing %s\n", buf);
    parseLine(lConfig, buf);
  }

  if (!lConfig->fileName || !lConfig->titleID || lConfig->type == DISC_TYPE_NONE) {
    printf("ERROR: Invalid configuration\n");
    freeConfig(lConfig);
    lConfig = NULL;
  }

  fclose(fd);
  unmountPFS();
  return lConfig;
}
