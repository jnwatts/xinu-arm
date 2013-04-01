#qemu-system-arm -M versatilepb -m 128M -nographic -cpu arm926 -kernel xinu.bin
qemu-system-arm -M raspi -m 128M -nographic -cpu arm1176 -kernel xinu.bin -gdb tcp::9087

