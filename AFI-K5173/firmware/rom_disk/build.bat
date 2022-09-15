as80 -x rom_disk.z80
crctool rom_disk.bin -p $ffff -s 0 -e $07fe -w $07fe
crctool rom_disk.bin -p $ffff -s $0800 -e $0ffe -w $0ffe
copy /b rom_disk.bin+disk.img rom.bin
