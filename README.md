# BBN Launcher

A simple OPL/Neutrino/POPS launcher based on [NHDDL](https://github.com/pcm720/nhddl) and CosmicScale's [OPL-Launcher-BDM](https://github.com/CosmicScale/OPL-Launcher-BDM).  
Designed to be used with the [PSBBN Definitive English Patch 2.0](https://github.com/CosmicScale/PSBBN-Definitive-English-Patch)

Supports Game ID for the Pixel FX line of products for both PS2 and PS1 titles.

## Supported backends

### __OPL__  
BBNL reads `hdd0:/__common/OPL/conf_hdd.cfg` to find OPL partition (`$OPL`) and launches `$OPL/OPNPS2LD.ELF` using `autoLaunchBDMGame` to load ISO from exFAT partition on internal HDD.  
If `conf_hdd.cfg` doesn't exist, BBNL falls back to `+OPL` or `__common` partitions.

### __Neutrino__  
BBNL launches Neutrino at `$OPL/neutrino/neutrino.elf` to load ISO from the exFAT partition on internal HDD.  
Supports reading [NHDDL's configuration files](https://github.com/pcm720/nhddl?tab=readme-ov-file#argument-files) from the exFAT partition for applying Neutrino arguments.

### POPS
BBNL loads a corresponding `POPStarter.elf` from the `__.POPS` partition.

**Note:**  
- __OPL__ backend requires OPL v1.2.0-Beta-2197-d5e0998 or later, which can be downloaded [here](https://github.com/ps2homebrew/Open-PS2-Loader/releases/tag/latest)
- __Neutrino__ backend requires Neutrino >1.5.0 with quickboot support, which can be downloaded [here](https://github.com/rickgaiser/neutrino/releases) _(temporarily not available)_

## Usage

Create a file named `launcher.cfg` and place the file in the same location as `bbnl.elf` or `BBNL.KELF`.

### OPL backend
```ini
file_name=Ridge Racer V.iso
title_id=SLUS_200.02
disc_type=CD
launcher=OPL
```

### Neutrino backend
```ini
file_name=Ecco the Dolphin.iso
title_id=SLUS_203.94
disc_type=DVD
launcher=NEUTRINO
```

### POPS backend
```ini
file_name=Ape Escape.VCD
title_id=SCUS_944.23
disc_type=POPS
launcher=POPS
```

All PS2 titles will be launched from the exFAT partition on the internal drive from the `DVD` directory for `disc_type=DVD` and from the `CD` directory for `disc_type=CD`.

All PS1 titles will be launched from `_.POPS` APA partition on the internal drive.  
Place the .VCD files on the internal drive in the `__.POPS` partition along with a renamed `POPStarter.elf` matching each .VCD file.
Make sure the renamed file ends with `.ELF` (in upper-case), not `.elf`.

### HDDOSD and PSBBN

You can create launcher partitions on the PS2 drive for compatibility with HDDOSD and PSBBN.  
To do this:
1. Create a 128 MB partition.
2. Inject `system.cnf`, `icon.sys` and `list.ico` into the partition header.

Example `system.cnf` file:

```ini
BOOT2 = pfs:/BBNL.KELF
VER = 1.00
VMODE = NTSC
HDDUNITPOWER = NICHDD
```

Prepare a signed executable (for example, by using [this app](https://www.psx-place.com/resources/kelftool-fmcb-compatible-fork.1104/))

```cmd
kelftool encrypt mbr bbnl.elf BBNL.KELF
```

Place `BBNL.KELF` and `launcher.cfg` files in the root of the partition.