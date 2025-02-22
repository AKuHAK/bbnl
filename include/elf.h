#ifndef _ELF_H_
#define _ELF_H_

// Launches the ELF, passing all arguments as-is.
// File name must be a relative path to ELF file.
int launchELF(char *fileName, int argc, ELFArgument *args);

#endif
