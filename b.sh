#!/bin/bash

CURDIR=`pwd`
export CROSS_COMPILE=$HOME/.bin/aarch64-none-elf-10.3/bin/aarch64-none-elf-

debug_flag=1

V=1

MK_PARAM="PLAT=${PLAT} DEBUG=${debug_flag} V=$V"


BOARD_COMMON_FEATURE='-nographic -machine virt,gic-version=3,secure=on, -cpu cortex-a55 -smp 1 -m 2G -d guest_errors,unimp' 
#-serial stdio 

function dumpdts
{
    qemu-system-aarch64 -machine virt,gic-version=3,secure=on,virtualization=off,dumpdtb=board.dtb -cpu cortex-a55 -nographic -smp 1 -m 2G
    dtc -I dtb -O dts board.dtb -o board.dts
}


while [[ $# -gt 0 ]]; do
    case $1 in
        release)
        debug_flag=0          
        shift
        ;;
        -c|clean)
        make clean
        shift
        ;;
        -a|all)
            make ${MK_PARAM} all
        shift
        ;;
        dump)
        dumpdts
        shift
        ;;
        run)
        make ${MK_PARAM} all
        qemu-system-aarch64 ${BOARD_COMMON_FEATURE} -bios ./main.bin -semihosting-config enable=on,target=native
        shift
        ;;
        debug)
        qemu-system-aarch64 ${BOARD_COMMON_FEATURE} -bios ./main.bin -semihosting-config enable=on,target=native -S -s
        shift
        ;;
        map)
        make ${MK_PARAM} memmap
        shift
        ;;
        *)
        make help
        shift
        ;;
    esac
done

