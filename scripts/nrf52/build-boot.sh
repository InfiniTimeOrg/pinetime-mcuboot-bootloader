#!/usr/bin/env bash
#  Build Mynewt bootloader on macOS and Linux

set -e  #  Exit when any command fails.
set -x  #  Echo all commands.

source paths.bash

#  Show the Arm Toolchain version.
arm-none-eabi-gcc --version

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
