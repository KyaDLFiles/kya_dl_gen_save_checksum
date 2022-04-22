# kya_dl_gen_save_checksum
Generates checksums for .dat file saves so the game accepts them as valid if tampered with
NOTE: slot_x.dat files (the actual save slots) seem to have additional checks yet to be defeated, so for now this is only truly useful for settings.dat

#Usage
gen_save_checksum(.exe) /path/to/save/file.dat
The checksums stored in the file will be fixed

#Documentation
https://kyadlfiles.github.io/technical/#save_header

GIGANTIC thanks to cherry from Discord for helping with this!
