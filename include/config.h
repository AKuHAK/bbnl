#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef enum {
  DISC_TYPE_NONE,
  DISC_TYPE_CD,  // PS2 CD Image
  DISC_TYPE_DVD, // PS2 DVD Image
  DISC_TYPE_POPS // PS1 POPS Image
} DiscType;

typedef enum {
  LAUNCHER_NONE,
  LAUNCHER_OPL,      // Open PS2 Loader
  LAUNCHER_NEUTRINO, // Neutrino
  LAUNCHER_POPS      // POPS
} Launcher;

typedef struct {
  char *fileName;
  char *titleID;
  DiscType type;
  Launcher launcher;
} LauncherConfig;

// Parses configuration file in the current working directory into LauncherConfig
// Returns NULL on failure
LauncherConfig *parseConfig();

// Releases memory used by config, including the passed pointer
void freeConfig(LauncherConfig *config);

#endif
