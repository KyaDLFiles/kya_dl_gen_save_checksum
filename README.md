# kya_dl_gen_save_checksum
Fixes the checksums inside Kya: Dark Lineage save files (and possibly other Eden Games games that uses the same libraries?) so that the game accepts them as valid
## Notice
Currently only supports the final NTSC and PAL builds of the game, as well as the [September 29 prototype](https://hiddenpalace.org/Kya:\_Dark\_Lineage\_\(Sep_29,\_2003\_prototype\))  
The [May 12 prototype](https://hiddenpalace.org/Kya:\_Dark\_Lineage\_\(May\_12,\_2003\_prototype\)) uses a slightly different format which the program currently doesn't support

# Usage
gensavecsum(.exe) /path/to/save/file.dat <--verbose>  
The checksums stored in the file will be fixed  
--verbose makes the program print the calculated checksums and data block sizes  
## Return codes
*0: ran succesfully*  
*1: error opening file*  
*2: missing argument*  
*3: file isn't a KDL save file*  
*4: bad file header, invalid section size value(s) (program has tried to read out of bounds)*  
*5: invalid argument(s)* 

# Documentation
https://kyadlfiles.github.io/technical/#save_header 

GIGANTIC thanks to [avail](https://github.com/avail) (_cherry_ on Discord) for helping with this!
