#ifndef _DEVICES_H_
#define _DEVICES_H_

#define BDM_MOUNTPOINT "mass0:"

// Waits for HDD modules to initialize
int initDevices();

// Attempts to mount PFS partition read-only
int mountPFS(char *partitionPath);

// Unmounts PFS partition mounted at pfs0:
void unmountPFS();

#endif
