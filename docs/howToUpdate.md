#How to update from the original MCUBoot bootloader to this one?
Since September 2020, PineTimes are running [InfiniTime 0.7.1](https://github.com/JF002/InfiniTime/releases/tag/0.7.1) and [Lup's MCUBoot bootloader](https://github.com/lupyuen/pinetime-rust-mynewt/releases/tag/v4.1.7).

InfiniTime can be easily and safely updated Over-The-Air using the DFU file. Users can even install other firmwares like WaspOS using the same OTA functionality.

Upgrading the bootloader is a more difficult task as there is no fallback. When the application firmware is updated, the bootloader is responsible to ensure that everything goes smoothly, and to recover in case of error. When we upgrade the bootloader, there is nothing else to ensure recovery in case of failure.

That's why I need to write this disclaimer : **Upgrading the bootloader is a risky operation that could brick your device in case of error (bad manipulation, software bug, hardware failure,...). Do it at your own risks and neither the contributors of this project or Pine64 or anyone else will be held accountable in case of failure during this operation!**

That being said, we did many tests to ensure that this update goes as smoothly as possible !

## Upgrade procedure 

### WARNINGS

 - Apply this procedure only from InfiniTime
 - Follow the following steps in this exact order
 - The recovery firmware is not compatible with the old bootloader. Install this recovery firmware only once the new bootloader is installed.

## Procedure

 - Install a relatively recent version of InfiniTime. I recommend >= 0.14 as BLE connectivity was improved in this version
 - Ensure the battery is charged, and put it on the charging cradle if possible.
 - OTA `reloader-mcuboot.zip`. This file contains a custom firmware that will overwrite the current bootloader with the new version.
 - The watch resets and the *old* bootloader swaps InfiniTime with this reloader tool.
 - The bootloader runs the reloader which overwrites the old bootloader with the new one and resets the watch when it's done.
 - The new bootloader is running! You'll notice that the boot logo changed : The green *PineTime* logo has been replaced by a white PineCone that progressively becomes green.
 - The bootloader reverts to InfiniTime (the version you were running just before) and loads     it.
 - OTA `pinetime-mcuboot-recovery-loader-0.14.1.zip` from InfiniTime repo. This custom firmware will install the recovery firmware into the external flash memory and reset the watch when it's done.
 - The bootloader reverts to InfiniTime again and loads it.

[**--> See it in video <--**](https://video.codingfield.com)

## What changed in this version since the previous version?

We fixed a few bugs that would temporarily *soft brick* the device. In some cases, the firmware and the bootloader would completely freeze, and the only way to work that issue around was to let the battery drain completely to rest the CPU.

This new version also provides a basic UI allowing the users to easily see when they are requesting a rollback or a recovery. 

The boot logo is also embedded into the bootloader binary instead of reading it from the external flash memory.

## How to use the bootloader

Most of the time, the bootloader works autonomously : it simply loads the firmware that is installed on the device (ex : InfiniTime).

When a new version of the firmware is OTA'ed, the bootloader automatically applies the updates. No user action is needed.

In normal operations, the bootloader displays a white pinecone that progressively becomes green. During that time (~5s), the user can request a firmware revert or recovery using the button:

 - Revert to the previous version of the firmware : press the button until the pinecone becomes blue. The bootloader will restart and reload the version of the firmware that was installed prior to the current one.
 - Load and run the recovery firmware : press the button until the pinecone becomes red. The bootloader will restart and run the recovery firmware. 

## How to use the recovery firmware

This firmware is a *light* version of InfiniTime that only provides the OTA functionality. You can use it in case of catastrophic failure to install a new firmware, when the bootloader is not able to run the firmware or revert to the previous version.