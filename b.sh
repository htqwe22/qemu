#!/bin/bash

CURDIR=`pwd`
export CROSS_COMPILE=$HOME/.bin/aarch64-none-elf-10.3/bin/aarch64-none-elf-

debug_flag=1

PLAT=qemu
V=0

MK_PARAM="PLAT=${PLAT} DEBUG=${debug_flag} V=$V"


BOARD_COMMON_FEATURE='-nographic -machine virt,gic-version=3,secure=on, -cpu cortex-a55 -smp 1 -m 2G -d guest_errors,unimp' 
#-serial stdio 

function dumpdts
{
    qemu-system-aarch64 -machine virt,gic-version=3,secure=on,virtualization=off,dumpdtb=board.dtb -cpu cortex-a55 -nographic -smp 1 -m 2G
    dtc -I dtb -O dts board.dtb -o board.dts
}


function address2line
{
    ${CROSS_COMPILE}addr2line -e main.elf $* -fipC
    #
    #-f：显示函数名
    #-i：如果地址是内联函数的地址，则输出最近的非内联函数的信息
    #-p：使输出更加人性化，每个地址的信息都打印在一行上
    #-s：仅显示文件名的基名，不显示完整路径
    #-C：将低级别的符号名解码为用户级别的名字

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
        addr)
        address2line $*
        break
        ;;
        *)
        make help
        shift
        ;;
    esac
done

#debug memory in vscode using gdb command 
# format -exec <command>; for example debug 0x600000d with 8word(8) using hex(x) with word(w) uinit
# -exec x/8xw 0x600000d
