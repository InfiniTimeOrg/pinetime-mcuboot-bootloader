#!/usr/bin/env bash
#  Build Mynewt bootloader on macOS and Linux

set -e  #  Exit when any command fails.
set -x  #  Echo all commands.

source paths.bash

#  Show the Arm Toolchain version.
arm-none-eabi-gcc --version

#  Apply patches
set +e
patch -uN -p1 --dry-run --silent -d repos/apache-mynewt-core/ < libs/pinetime_boot/patches/01-spiflash.patch 2>/dev/null

#If the patch has not been applied then the $? which is the exit status
#for last command would have a success status code = 0
if [ $? -eq 0 ];
then
    #apply the patch
    patch -uN -p1 -d repos/apache-mynewt-core/ < libs/pinetime_boot/patches/01-spiflash.patch
fi
set -e


#  Build the bootloader.
newt build nrf52_boot

#  Show the size.
newt size -v nrf52_boot

arm-none-eabi-objcopy -I binary -O ihex bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf.bin bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf.hex
scripts/hex2c.py bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf.hex > reloader/src/boards/pinetime/bootloader.h
make -C reloader BOARD=pinetime

set +x
echo "Bootloader firmware (elf) : bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf"
echo "Bootloader firmware (bin) : bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.bin"
echo "Bootloader firmware (hex) : bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.hex"
echo "Reloader (DFU) : reloader/build-pinetime/reloader-mcuboot.zip"
