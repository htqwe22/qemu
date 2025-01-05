

export CROSS_COMPILE=$HOME/.bin/aarch64-none-elf-10.3/bin/aarch64-none-elf-
GDB=${CROSS_COMPILE}gdb
$GDB --tui main.elf


#target remote localhost:1234

