#ifndef __PINETIME_FACTORY_H__
#define __PINETIME_FACTORY_H__

/// Copy the recovery firmware from the external SPI Flash memory to the secondary slot.
/// It'll be installed in the primary slot by MCUBoot.
void restore_factory(void);


#endif