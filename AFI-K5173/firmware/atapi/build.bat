as80 -x atapi.z80
crctool atapi.bin -p $ffff -s 0 -e $07fe -w $07fe
crctool atapi.bin -p $ffff -s $0800 -e $0ffe -w $0ffe
