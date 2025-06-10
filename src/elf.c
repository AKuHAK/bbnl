#include "common.h"
#include "config.h"
#include "loader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// To inject signature from embedded buffer (signature.bin)
int injectHDDSignatureFromBuffer(char *buffer)
{
    int result;
    iox_stat_t statFile;
    unsigned int numSectors;
    unsigned int sector;
    unsigned int remainder;
    unsigned int i;
    unsigned int numBytes;
    hddSetOsdMBR_t MBRInfo;

    // Buffer for I/O devctl operations on HDD
    uint8_t IOBuffer[512 + sizeof(hddAtaTransfer_t)] __attribute__((aligned(64)));

    // IF stats can be retrieved
    if ((result = fileXioGetStat("hdd0:__net", &statFile)) >= 0) {
        // Sector to write
        sector = statFile.private_5 + JP_signature_offset;

        // Total sectors to inject
        numSectors = 2;
        numBytes = 512;

        // Writes sectors
        for (i = 0; i < numSectors; i++) {
            // Copies from buffer
            memcpy(IOBuffer + sizeof(hddAtaTransfer_t), buffer + 512 * i, numBytes);
            // Performs write operation for one sector
            ((hddAtaTransfer_t *)IOBuffer)->lba = sector + i;
            ((hddAtaTransfer_t *)IOBuffer)->size = 1;

            if ((result = fileXioDevctl("hdd0:", APA_DEVCTL_ATA_WRITE, IOBuffer, 512 + sizeof(hddAtaTransfer_t), NULL, 0)) < 0)
                break;
        }
    }
    return result;
}

// Launches the ELF, passing all arguments as-is
// File name must be a relative path to ELF file.
int launchELF(char *fileName, int argc, ELFArgument *args) {
  char *filePath = malloc(sizeof(PFS_MOUNTPOINT) + strlen(fileName));
  strcpy(filePath, PFS_MOUNTPOINT);
  strcat(filePath, fileName);

  if (tryFile(filePath)) {
    printf("ERROR: %s is inaccessible\n", filePath);
    return -ENOENT;
  }

  FILE *fd = fopen("pfs0:/signature.bin", "rb");
  if (fd) { // if file exists, inject HDD signature
    injectHDDSignatureFromBuffer(fd);
    printf("Injected HDD signature from pfs0:/signature.bin\n");
    fclose(fd);
  }
  argc += 1;
  char **argv = malloc(argc * sizeof(char *));

  printf("Launching %s with %d arguments:\n", filePath, argc);
  // Assemble arguments
  ELFArgument *arg = args;
  ELFArgument *narg = NULL;

  for (int i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, arg->arg);
    argv[i] = arg->arg;

    narg = arg->next;
    free(arg);

    if (!narg)
      break;

    arg = narg;
  }

  printf("ERROR: Failed to load %s: %d\n", argv[0], LoadELFFromFile(argc, argv));
  return -ENOENT;
}

