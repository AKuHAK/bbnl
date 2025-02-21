#include "config.h"
#include "common.h"
#include <ps2sdkapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char pfsPostfixStr[] = ":pfs:";
static char apaPartitionPrefix[] = "hdd0:PP.";
static char bbnlCfgPrefix[] = BDM_MOUNTPOINT "/bbnl/";

// Parses the line into LauncherConfig
void parseLine(LauncherConfig *info, char *line);

// Gets configuration file name from the current working directory and parses it into LauncherConfig
// Returns NULL on failure
LauncherConfig *parseConfig() {
  char buf[PATH_MAX];

  // Get the current working directory
  if (getcwd(buf, PATH_MAX) == NULL) {
    printf("ERROR: Failed to get CWD\n");
    return NULL;
  }
  printf("CWD is %s\n", buf);

  // Make sure CWD is valid
  char *pfsPostfix = strstr(buf, pfsPostfixStr);
  if (!pfsPostfix || strncmp(buf, apaPartitionPrefix, sizeof(apaPartitionPrefix) - 1)) {
    printf("ERROR: Unsupported partition name\n");
    return NULL;
  }

  // Replace PFS postfix from the current working directory path with ".cfg"
  strcpy(pfsPostfix, ".cfg");

  // Partition name without the APA partition prefix
  char *cfgName = buf + sizeof(apaPartitionPrefix) - 1;

  // Generate config path
  char *cfgPath = malloc(sizeof(bbnlCfgPrefix) + strlen(cfgName));
  strcpy(cfgPath, bbnlCfgPrefix);
  strcat(cfgPath, cfgName);

  printf("Loading configuration file from %s\n", cfgPath);
  // Open the configuration file
  FILE *fd = fopen(cfgPath, "rb");
  free(cfgPath);
  if (!fd) {
    printf("ERROR: Failed to open the configuration file\n");
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
    parseLine(lConfig, buf);
  }

  if (!lConfig->fileName || !lConfig->titleID || lConfig->type == DISC_TYPE_NONE) {
    printf("ERROR: Invalid configuration\n");
    freeConfig(lConfig);
    lConfig = NULL;
  }

  fclose(fd);
  return lConfig;
}

// Releases memory used by config, including the passed pointer
void freeConfig(LauncherConfig *config) {
  if (!config->fileName)
    free(config->fileName);

  if (!config->titleID)
    free(config->titleID);

  free(config);
}

// Parses the line into LauncherConfig
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
    printf("File name: %s\n", val);
    info->fileName = strdup(val);
    return;
  }
  if (!strcmp(line, "title_id")) {
    printf("Title ID: %s\n", val);
    info->titleID = strdup(val);
    return;
  }
  if (!strcmp(line, "disc_type")) {
    printf("Disc type: %s\n", val);
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
    printf("Launcher: %s\n", val);
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
