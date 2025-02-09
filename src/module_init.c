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
IRX_DEFINE(ps2hdd);
IRX_DEFINE(ps2fs);

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

// Initializes PS2HDD-BDM arguments
char *initPS2HDDArguments(uint32_t *argLength);
// Initializes PS2FS arguments
char *initPS2FSArguments(uint32_t *argLength);

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
    // PS2HDD
    INT_MODULE(ps2hdd, &initPS2HDDArguments),
    // PFS
    INT_MODULE(ps2fs, &initPS2FSArguments),
};
#define MODULE_COUNT sizeof(moduleList) / sizeof(ModuleListEntry)

// Loads module, executing argument function if it's present
int loadModule(ModuleListEntry *mod);
#include <unistd.h>
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
    if (!strcmp(moduleList[i].name, "ata_bd"))
      sleep(1); // PS2 APA modules can hang forever if atad is not given enough time to initialize

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

// up to 4 descriptors, 20 buffers
static char ps2hddArguments[] = "-o"
                                "\0"
                                "4"
                                "\0"
                                "-n"
                                "\0"
                                "20";
// Sets arguments for PS2HDD modules
char *initPS2HDDArguments(uint32_t *argLength) {
  *argLength = sizeof(ps2hddArguments);

  char *argStr = malloc(sizeof(ps2hddArguments));
  memcpy(argStr, ps2hddArguments, sizeof(ps2hddArguments));
  return argStr;
}

// up to 10 descriptors, 40 buffers
char ps2fsArguments[] = "-o"
                        "\0"
                        "10"
                        "\0"
                        "-n"
                        "\0"
                        "40";
// Sets arguments for PS2HDD modules
char *initPS2FSArguments(uint32_t *argLength) {
  *argLength = sizeof(ps2fsArguments);

  char *argStr = malloc(sizeof(ps2fsArguments));
  memcpy(argStr, ps2fsArguments, sizeof(ps2fsArguments));
  return argStr;
}
