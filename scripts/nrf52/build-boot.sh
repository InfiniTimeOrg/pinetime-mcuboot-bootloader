#!/usr/bin/env bash
#  Build Mynewt bootloader on macOS and Linux

TARGET=${1:-pinetime}
echo "Building target $TARGET"

set -e  #  Exit when any command fails.
set -x  #  Echo all commands.

source paths.bash

#  Show the Arm Toolchain version.
arm-none-eabi-gcc --version

#  Build the bootloader.
newt build $TARGET

#  Show the size.
newt size -v $TARGET

arm-none-eabi-objcopy -I binary -O ihex bin/targets/$TARGET/app/@mcuboot/boot/mynewt/mynewt.elf.bin bin/targets/$TARGET/app/@mcuboot/boot/mynewt/mynewt.elf.hex
scripts/hex2c.py bin/targets/$TARGET/app/@mcuboot/boot/mynewt/mynewt.elf.hex > reloader/src/boards/$TARGET/bootloader.h
make -C reloader build-$TARGET/reloader-mcuboot.zip all BOARD=$TARGET

set +x
echo "Bootloader firmware (elf) : bin/targets/$TARGET/app/@mcuboot/boot/mynewt/mynewt.elf"
echo "Bootloader firmware (bin) : bin/targets/$TARGET/app/@mcuboot/boot/mynewt/mynewt.bin"
echo "Bootloader firmware (hex) : bin/targets/$TARGET/app/@mcuboot/boot/mynewt/mynewt.hex"
echo "Reloader (DFU) : reloader/build-$TARGET/reloader-mcuboot.zip"
