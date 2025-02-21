#include "module_init.h"
#include <iopcontrol.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macros for loading embedded IOP modules
#define IRX_DEFINE(mod)                                                                                                                              \
  extern unsigned char mod##_irx[] __attribute__((aligned(16)));                                                                                     \
  extern uint32_t size_##mod##_irx

// Defines moduleList entry for embedded module
#define INT_MODULE(mod, argFunc) {#mod, mod##_irx, &size_##mod##_irx, 0, NULL, argFunc}

// Embedded IOP modules
IRX_DEFINE(iomanX);
IRX_DEFINE(fileXio);
IRX_DEFINE(ps2dev9);
IRX_DEFINE(bdm);
IRX_DEFINE(bdmfs_fatfs);
IRX_DEFINE(ata_bd);

// Function used to initialize module arguments.
// Must set argLength and return non-null pointer to a argument string if successful.
// Returned pointer must point to dynamically allocated memory
typedef char *(*moduleArgFunc)(uint32_t *argLength);

typedef struct ModuleListEntry {
  char *name;                     // Module name
  unsigned char *irx;             // Pointer to IRX module
  uint32_t *size;                 // IRX size. Uses pointer to avoid compilation issues with internal modules
  uint32_t argLength;             // Total length of argument string
  char *argStr;                   // Module arguments
  moduleArgFunc argumentFunction; // Function used to initialize module arguments
} ModuleListEntry;

// List of modules to load
static ModuleListEntry moduleList[] = {
    INT_MODULE(iomanX, NULL),
    INT_MODULE(fileXio, NULL),
    // DEV9
    INT_MODULE(ps2dev9, NULL),
    // BDM
    INT_MODULE(bdm, NULL),
    // FAT/exFAT
    INT_MODULE(bdmfs_fatfs, NULL),
    // ATA
    INT_MODULE(ata_bd, NULL),
};
#define MODULE_COUNT sizeof(moduleList) / sizeof(ModuleListEntry)

// Loads module, executing argument function if it's present
int loadModule(ModuleListEntry *mod);

// Initializes IOP modules
int initModules() {
  int ret = 0;

  // Skip rebooting IOP if modules were loaded previously
  printf("Rebooting IOP\n");
  while (!SifIopReset("", 0)) {
  };
  while (!SifIopSync()) {
  };

  // Initialize the RPC manager
  SifInitRpc(0);

  // Apply patches required to load modules from EE RAM
  if ((ret = sbv_patch_enable_lmb()))
    return ret;
  if ((ret = sbv_patch_disable_prefix_check()))
    return ret;

  // Load modules
  for (int i = 0; i < MODULE_COUNT; i++) {
    if ((ret = loadModule(&moduleList[i]))) {
      printf("ERROR: Failed to initialize module %s: %d\n", moduleList[i].name, ret);
      return ret;
    }

    // Clean up arguments
    if (moduleList[i].argStr != NULL)
      free(moduleList[i].argStr);
  }
  return 0;
}

// Loads module, executing argument function if it's present
int loadModule(ModuleListEntry *mod) {
  int ret, iopret = 0;

  printf("Loading %s\n", mod->name);

  // If module has an arugment function, execute it
  if (mod->argumentFunction != NULL) {
    mod->argStr = mod->argumentFunction(&mod->argLength);
    if (mod->argStr == NULL) {
      return -1;
    }
  }

  ret = SifExecModuleBuffer(mod->irx, *mod->size, mod->argLength, mod->argStr, &iopret);
  if (ret >= 0)
    ret = 0;
  if (iopret == 1)
    ret = iopret;

  return ret;
}
