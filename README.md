# BBN Launcher

A simple OPL/Neutrino/POPS launcher based on [NHDDL](https://github.com/pcm720/nhddl) and CosmicScale's [OPL-Launcher-BDM](https://github.com/CosmicScale/OPL-Launcher-BDM).  
Designed to be used with the [PSBBN Definitive English Patch 2.0](https://github.com/CosmicScale/PSBBN-Definitive-English-Patch)

Supports Game ID for the Pixel FX line of products for both PS2 and PS1 titles.

## Supported backends

BBNL uses APA partition name to search for config file on the exFAT partition.
`hdd0:PP.` prefix and `:pfs:` postfix are removed from the current working directory and the resulting name is used to look for config in `mass0:/bbnl/`.
For example, for `hdd0:PP.SLUS-20002.RIDGE_RACER_V:pfs:` it will try to open `mass0:/bbnl/SLUS-20002.RIDGE_RACER_V.cfg`.

### __OPL__  
If launcher is set to `OPL`, BBNL will launch `mass0:/bbnl/OPNPS2LD.ELF` using `autoLaunchBDMGame` to load ISO from exFAT partition on internal HDD.  

### __Neutrino__  
If launcher is set to `NEUTRINO`, BBNL will launch `mass0:/neutrino/neutrino.elf` to load ISO from the exFAT partition on internal HDD.  
Supports reading [NHDDL's configuration files](https://github.com/pcm720/nhddl?tab=readme-ov-file#argument-files) from the exFAT partition for applying Neutrino arguments.

### POPS
If launcher or disc type are set to `POPS`, BBNL will launch `mass0:/bbnl/POPSTARTER.ELF` and pass `bbnl:<file_name without .VCD>.ELF` to the POPStarter.
POPStarter will ignore the `bbnl:` prefix and assume that it needs to boot `hdd0:__POPS/<file_name>`.

**Note:**  
- __OPL__ backend requires OPL v1.2.0-Beta-2197-d5e0998 or later, which can be downloaded [here](https://github.com/ps2homebrew/Open-PS2-Loader/releases/tag/latest)
- __Neutrino__ backend requires Neutrino >1.5.0 with quickboot support, which can be downloaded [here](https://github.com/rickgaiser/neutrino/releases) _(temporarily not available)_

## Usage

Create a file named `<APA partition name without PP.>.cfg` and place the file in `mass0:/bbnl/`.

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
Place the .VCD files on the internal drive in the `__.POPS` partition.
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