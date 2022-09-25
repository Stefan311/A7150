as80 -x ide.z80
crctool ide.bin -p $ffff -s 0 -e $07fe -w $07fe
crctool ide.bin -p $ffff -s $0800 -e $0ffe -w $0ffe
