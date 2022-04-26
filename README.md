# kya_dl_gen_save_checksum
Fixes the checksums inside Kya: Dark Lineage save files (and possibly other Eden Games games that uses the same libraries?) so that the game accepts them as valid
## Notice
Currently only supports the final NTSC and PAL builds of the game, as well as the [September 29 prototype](https://hiddenpalace.org/Kya:\_Dark\_Lineage\_\(Sep_29,\_2003\_prototype\))  
The [May 12 prototype](https://hiddenpalace.org/Kya:\_Dark\_Lineage\_\(May\_12,\_2003\_prototype\)) uses a slightly different format which the program currently doesn't support

# Usage
gensavecsum(.exe) /path/to/save/file.dat <--verbose>
The checksums stored in the file will be fixed
The --verbose makes the program print the calculated checksums

# Documentation
https://kyadlfiles.github.io/technical/#save_header

GIGANTIC thanks to [avail](https://github.com/avail) (_cherry_ on Discord) for helping with this!
