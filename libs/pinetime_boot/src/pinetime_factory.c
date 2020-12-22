#include "pinetime_boot/pinetime_factory.h"
#include <hal/hal_flash.h>
#include "os/mynewt.h"

//  Flash Device for Image
#define FLASH_DEVICE 1  //  0 for Internal Flash ROM, 1 for External SPI Flash

/// Buffer for reading flash and writing to display
#define BATCH_SIZE  256  //  Max number of SPI data bytes to be transmitted
static uint8_t flash_buffer[BATCH_SIZE];

#define FACTORY_SIZE 0x40000
#define FACTORY_OFFSET_SOURCE 0
#define FACTORY_OFFSET_DESTINATION 0x40000


void restore_factory(void) {
  int rc;
  for (uint32_t erased = 0; erased < FACTORY_SIZE; erased += 0x1000) {
    rc = hal_flash_erase_sector(FLASH_DEVICE, FACTORY_OFFSET_DESTINATION + erased);
  }

  for(uint32_t offset = 0; offset < FACTORY_SIZE; offset += BATCH_SIZE) {
    rc = hal_flash_read(FLASH_DEVICE, FACTORY_OFFSET_SOURCE + offset, flash_buffer, BATCH_SIZE);
    assert(rc == 0);
    rc = hal_flash_write(FLASH_DEVICE, FACTORY_OFFSET_DESTINATION + offset, flash_buffer, BATCH_SIZE);
    assert(rc == 0);
  }
}
