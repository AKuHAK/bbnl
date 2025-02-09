#ifndef _OPL_H_
#define _OPL_H_

// Launches the ISO via OPL.
// Assumes OPL partition is mounted.
int launchOPL(char *filePath, char *titleID, DiscType mediaType);

// Mounts OPL partition
int mountOPLPartition();

#endif
