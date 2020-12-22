# Bootloader for PineTime based on MyNEWT and MCUBoot

## Features

- Start the **application** firmware
- Provide a safe and efficient way to **swap firmwares** from the secondary slot (in external flash memory) and the primary slot (in internal flash memory), thanks to MCUBoot 
- **Revert** to the previous version of the firmware
- Load and run a **recovery** firmware
- Basic UI (logo, progress bar, button)

## Memory map

![Memory Map](docs/pictures/memoryMap.png "Memory map")

The PineTime is based on 2 flash memories:
 - **The internal flash** (512KB) : this flash is integrated into the MCU. The MCU runs the code from this memory. It cannot run codes directly from the external SPI flash memory. This internal memory contains the bootloader code as well as the application firmware. The scratch area is used by MCUBoot to swap firmwares from internal and external memories.
 - **The external** flash (4MB) : this memory is external to the MCU and is connected to the MCU using an SPI bus. It contains the recovery firmware (in the section *Bootloader Assets*) and the secondary slot for MCUBoot (*OTA section*). The *FS* part is available for the application firmware.

## Boot flow

The bootloader is the first piece of software that is running on the PineTime. It's main goal is to load the application firmware. It is also responsible to swap the firmware from the secondary and primary slot if a newer version of the firmware is present in the secondary slot. It also provides the possibility to revert to the previous version of the firmware and to restore a recovery firmware that supports OTA.

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

