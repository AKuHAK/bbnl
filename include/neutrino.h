#ifndef _NEUTRINO_H_
#define _NEUTRINO_H_

#include "config.h"

// Launches the ISO via Neutrino.
// Assumes parititon containing Neutrino as mounted as pfs0.
int launchNeutrino(char *fileName, DiscType type);

#endif
