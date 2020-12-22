#include <os/os.h>

#include "pinetime_boot/pinetime_delay.h"

void pinetime_delay_us(uint32_t time_us) {
#define NRFX_COREDEP_DELAY_US_LOOP_CYCLES  3
#define NRFX_DELAY_CPU_FREQ_MHZ 64
  __ALIGN(16)
  static const uint16_t delay_machine_code[] = {
          0x3800 + NRFX_COREDEP_DELAY_US_LOOP_CYCLES, // SUBS r0, #loop_cycles
          0xd8fd, // BHI .-2
          0x4770  // BX LR
  };

  typedef void (* delay_func_t)(uint32_t);
  const delay_func_t delay_cycles =
          // Set LSB to 1 to execute the code in the Thumb mode.
          (delay_func_t)((((uint32_t)delay_machine_code) | 1));
  uint32_t cycles = time_us * NRFX_DELAY_CPU_FREQ_MHZ;
  delay_cycles(cycles);
}

/// Sleep for the specified number of milliseconds
void pinetime_delay_ms(uint32_t ms) {
#if MYNEWT_VAL(OS_SCHEDULING)  //  If Task Scheduler is enabled (i.e. not MCUBoot)...
  uint32_t delay_ticks = ms * OS_TICKS_PER_SEC / 1000;
    os_time_delay(delay_ticks);
#else  //  If Task Scheduler is disabled (i.e. MCUBoot)...

  do {
    pinetime_delay_us(1000);
  } while (--ms);

#endif  //  MYNEWT_VAL(OS_SCHEDULING)
}
