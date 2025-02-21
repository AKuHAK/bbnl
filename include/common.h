#ifndef _COMMON_H_
#define _COMMON_H_

// exFAT on internal HDD
#define BDM_MOUNTPOINT "mass0:"

// Tests if file exists by opening it
int tryFile(char *filePath);

#endif
