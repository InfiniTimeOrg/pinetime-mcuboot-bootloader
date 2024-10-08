# PineTime MCUBoot bootloader
An open source bootloader for the PineTime based on MyNEWT and MCUBoot.

## About this project
This repo is based on [Lupyuen's](https://github.com/lupyuen) [amazing work](https://github.com/lupyuen/pinetime-rust-mynewt).

Lup is the very first developer to have dared to build a bootloader for the PineTime. And it was a success as [version 4.1.7](https://github.com/lupyuen/pinetime-rust-mynewt/releases/tag/v4.1.7) was preloaded in the new batch of PineTime devkits in September 2020.

Lup uses [his repo](https://github.com/lupyuen/pinetime-rust-mynewt) to conduct many experiments (Rust firmware, mynewt, lvgl,...). In this the, the code of the bootloader was mixed with a lot of other code, and the history was not easy to read.

That's why I decided (with his agreement) to extract the bootloader code into a shiny new project. This will ease the code readability and its maintenance.

Changes between the original repo and the "Initial commit" of this one can be seen in this fork : https://github.com/JF002/pinetime-rust-mynewt/tree/restore-factory-image.

## Features

- Start the **application** firmware
- Provide a safe and efficient way to **swap firmwares** from the secondary slot (in external flash memory) and the primary slot (in internal flash memory), thanks to MCUBoot
- **Revert** to the previous version of the firmware
- Load and run a **recovery** firmware
- Basic UI (logo, progress bar, button)

## Update

Please see [this page](docs/howToUpdate.md) for more info about the update procedure.

## Memory map

![Memory Map](docs/pictures/memoryMap.png "Memory map")

The PineTime is based on 2 flash memories:
 - **The internal flash** (512KB) : this flash is integrated into the MCU. The MCU runs the code from this memory. It cannot run codes directly from the external SPI flash memory. It contains the following sections:
   - **Bootloader** (28KB - 0x7000B) : This bootloader.
   - **Log** (4KB - 0x1000B) : Space reserved for boot/error logs. Not currently useed.
   - **Application firmware** (464KB - 0x74000B) : application wrapped into a MCUBoot image (header, TLV, trailer).
   - **Scrach** (4KB - 0x1000B) : the scratch area that allows MCUBoot to swap firmware between the internal and external memories.
   - **Spare** (12KB - 0x3000B) : a spare and unused area.
 - **The external** flash (4MB) : this memory is external to the MCU and is connected to the MCU using an SPI bus. It contains the recovery firmware (in the section *Bootloader Assets*) and the secondary slot for MCUBoot (*OTA section*). The *FS* part is available for the application firmware.

## Boot flow

The bootloader is the first piece of software that is running on the PineTime. Its main goal is to load the application firmware. It is also responsible to swap the firmware from the secondary and primary slot if a newer version of the firmware is present in the secondary slot. It also provides the possibility to revert to the previous version of the firmware and to restore a recovery firmware that supports OTA.

![Boot flow](docs/pictures/workflow.png "Boot flow")


At startup, the bootloader displays a **logo and its version**:

![Bootloader start](docs/pictures/bootloader_start.png "Bootloader start")

Then, it waits for ~5s in case the user presses the button. The **progress** is shown by the color of the logo.

When the user just wants to **run the current firmware**, they won't touch the button, and the logo will be filled in **green**:

![Bootloader normal boot](docs/pictures/bootloader_normal_boot.png "Bootloader normal boot")

When the timeout is elapsed, **MCUBoot** will be run. It'll swap the firmware if needed and then run the firmware from the primary slot.

If the user presses the button until the logo is drawn in **blue**, the previous version of the firmware will be **reverted**:

![Bootloader revert](docs/pictures/bootloader_revert.png "Bootloader revert")

If the user presses the button longer, until the logo is drawn in **red**, the **recovery** firmware will be loaded into the primary slot and will be run after the next reset:

![Bootloader recovery](docs/pictures/bootloader_recovery.png "Bootloader recovery")

## Recovery firmware

The recovery firmware is a "lightweight" version of InfiniTime. It is stripped of most of its functionalities : it only provides **basic UI, BLE connectivity and OTA**.

The goal of this firmware is to provide a mean for the user to OTA a new firmware in case the current firmware does not work properly or does not provide OTA.

![Bootloader recovery OTA](docs/pictures/bootloader_recovery_ota.png "Bootloader recovery OTA")


## How to build

- Install `newt` tool
- Clone the project and `cd` into it
- Init and update submodules : `git submodule update --init --recursive`
- Configure mynewt : `newt upgrade`
- Build : `scripts/nrf52/build-boot.sh`. The firmware is generated in `bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf` and the DFU file for the reloader : `reloader/build-pinetime/reloader-mcuboot.zip`

### Bitmap generation

This project uses 2 bitmaps (defined in [graphic.h](libs/pinetime_boot/src/graphic.h)) : 
 - the Pine logo
 - the version indicator

To reduce the amount of flash memory needed to store them, they are compressed using a simple [RLE encoding](https://en.wikipedia.org/wiki/Run-length_encoding).

Use the tool [rle_encode.py](tools/rle_encode.py) to convert a png file into a RLE encoded buffer:

```shell
python tools/rle_encode.py --c libs/pinetime_boot/src/version-1.0.1.png
```

Here is the corresponding output : 

```
// 1-bit RLE, generated from libs/pinetime_boot/src/version-1.0.1.png, 269 bytes
static const uint8_t version-1.0.1[] = {
  0x59, 0x56, 0x2, 0x56, 0x2, 0x56, 0x2, 0x56, 0x2, 0x2, 0x2, 0xd,
  0x2, 0x5, 0x4, 0x15, 0x5, 0x12, 0x4, 0xa, 0x2, 0x3, 0x2, 0xb,
  0x2, 0x4, 0x6, 0x13, 0x9, 0xe, 0x6, 0xa, 0x2, 0x3, 0x2, 0xb,
  0x2, 0x4, 0x2, 0x2, 0x2, 0x13, 0x2, 0x5, 0x2, 0xe, 0x2, 0x2,
  0x2, 0xa, 0x2, 0x3, 0x3, 0x9, 0x3, 0x8, 0x2, 0x12, 0x2, 0x7,
  0x2, 0x11, 0x2, 0xa, 0x2, 0x4, 0x2, 0x9, 0x2, 0x9, 0x2, 0x12,
  0x2, 0x7, 0x2, 0x11, 0x2, 0xa, 0x2, 0x4, 0x2, 0x9, 0x2, 0x9,
  0x2, 0x11, 0x2, 0x9, 0x2, 0x10, 0x2, 0xa, 0x2, 0x5, 0x2, 0x7,
  0x2, 0xa, 0x2, 0x11, 0x2, 0x9, 0x2, 0x10, 0x2, 0xa, 0x2, 0x5,
  0x2, 0x7, 0x2, 0xa, 0x2, 0x11, 0x2, 0x9, 0x2, 0x10, 0x2, 0xa,
  0x2, 0x5, 0x3, 0x5, 0x3, 0xa, 0x2, 0x11, 0x2, 0x9, 0x2, 0x10,
  0x2, 0xa, 0x2, 0x6, 0x2, 0x5, 0x2, 0xb, 0x2, 0x11, 0x2, 0x9,
  0x2, 0x10, 0x2, 0xa, 0x2, 0x6, 0x2, 0x5, 0x2, 0xb, 0x2, 0x11,
  0x2, 0x9, 0x2, 0x10, 0x2, 0xa, 0x2, 0x7, 0x2, 0x3, 0x2, 0xc,
  0x2, 0x11, 0x2, 0x9, 0x2, 0x10, 0x2, 0xa, 0x2, 0x7, 0x2, 0x3,
  0x2, 0xc, 0x2, 0x11, 0x2, 0x9, 0x2, 0x10, 0x2, 0xa, 0x2, 0x7,
  0x2, 0x3, 0x2, 0xc, 0x2, 0x12, 0x2, 0x7, 0x2, 0x11, 0x2, 0xa,
  0x2, 0x8, 0x2, 0x1, 0x2, 0xd, 0x2, 0x12, 0x2, 0x7, 0x2, 0x11,
  0x2, 0xa, 0x2, 0x8, 0x2, 0x1, 0x2, 0xd, 0x2, 0xa, 0x2, 0x7,
  0x2, 0x5, 0x2, 0x6, 0x2, 0xa, 0x2, 0xa, 0x2, 0x8, 0x5, 0x9,
  0xa, 0x6, 0x2, 0x7, 0x9, 0x6, 0x2, 0x6, 0xa, 0x6, 0x2, 0x9,
  0x3, 0xa, 0xa, 0x6, 0x2, 0x9, 0x5, 0x8, 0x2, 0x6, 0xa, 0x6,
  0x2, 0x56, 0x2, 0x56, 0x59,
};
```

You can then copy/paste this C array to the corresponding definition in [graphic.h](libs/pinetime_boot/src/graphic.h). 

## About the code

This project is based on MyNEWT RTOS and MCUBoot bootloader. The specific code for the PineTime is located in `libs/pinetime_boot`.

# Patches

 - [01-spiflash.patch](libs/pinetime_boot/patches/01-spiflash.patch) - July 2024 : Add support for the new SPI Flash memory chip (BY25Q32) into the `spiflash` driver of MyNewt. See [this issue](https://github.com/InfiniTimeOrg/pinetime-mcuboot-bootloader/issues/11) for more information.