as80 -x ide_chs.z80
crctool ide_chs.bin -p $ffff -s 0 -e $07fe -w $07fe
crctool ide_chs.bin -p $ffff -s $0800 -e $0ffe -w $0ffe
