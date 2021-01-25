#include "pinetime_boot/version.h"
#include <hal/nrf_timer.h>

void pinetime_set_version(void) {
  /* Store the bootloader version into NRF_TIMER2->CC[0]
   * The application needs to read this register before using TIMER2
   * This is how adafruit-bootloader exports its version to the application */
  NRF_TIMER2->CC[0] = PINETIME_BOOTLOADER_VERSION;
}
