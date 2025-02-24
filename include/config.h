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
  LAUNCHER_POPS,     // POPS
  LAUNCHER_ELF       // Generic ELF
} Launcher;

typedef struct ELFArgument {
  char *arg;                // Argument
  struct ELFArgument *next; // Next argument in the list
} ELFArgument;

typedef struct {
  char *fileName;
  char *titleID;
  DiscType type;
  Launcher launcher;
  ELFArgument *args; // A linked list of optional ELF arguments
  int argCount;      // Number of optional ELF arguments in the list
} LauncherConfig;

// Generates config name from APA partition path
// and parses configuration file from the exFAT partition into LauncherConfig
// Returns NULL on failure
LauncherConfig *parseConfig(char *partitionPath);

// Releases memory used by config, including the passed pointer
void freeConfig(LauncherConfig *config);

#endif
