/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
//  Display image on ST7789 display controller (240 x 240)
#include <inttypes.h>
#include "os/mynewt.h"
#include <console/console.h>
#include <hal/hal_bsp.h>
#include <hal/hal_flash.h>
#include <hal/hal_flash_int.h>
#include <hal/hal_gpio.h>
#include <hal/hal_spi.h>
#include <stdio.h>
#include <string.h>
#include "pinetime_boot/pinetime_boot.h"
#include "pinetime_boot/pinetime_delay.h"
#include "graphic.h"
//  GPIO Pins. From rust\piet-embedded\piet-embedded-graphics\src\display.rs
#define DISPLAY_SPI   0  //  Mynewt SPI port 0
#define DISPLAY_CS   25  //  LCD_CS (P0.25): Chip select
#define DISPLAY_DC   18  //  LCD_RS (P0.18): Clock/data pin (CD)
#define DISPLAY_RST  26  //  LCD_RESET (P0.26): Display reset
#define DISPLAY_HIGH 23  //  LCD_BACKLIGHT_{LOW,MID,HIGH} (P0.14, 22, 23): Backlight (active low)
#define BATCH_SIZE  256  //  Max number of SPI data bytes to be transmitted

//  Screen Size
#define ROW_COUNT 240
#define COL_COUNT 240
#define BYTES_PER_PIXEL 2

//  ST7789 Colour Settings
#define INVERTED 1  //  Display colours are inverted
#define RGB      1  //  Display colours are RGB

//  Flash Device for Image
#define FLASH_DEVICE 1  //  0 for Internal Flash ROM, 1 for External SPI Flash

//  ST7789 Commands. From https://github.com/lupyuen/st7735-lcd-batch-rs/blob/master/src/instruction.rs
#define NOP 0x00
#define SWRESET 0x01
#define RDDID 0x04
#define RDDST 0x09
#define SLPIN 0x10
#define SLPOUT 0x11
#define PTLON 0x12
#define NORON 0x13
#define INVOFF 0x20
#define INVON 0x21
#define DISPOFF 0x28
#define DISPON 0x29
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C
#define RAMRD 0x2E
#define PTLAR 0x30
#define COLMOD 0x3A
#define MADCTL 0x36
#define FRMCTR1 0xB1
#define FRMCTR2 0xB2
#define FRMCTR3 0xB3
#define INVCTR 0xB4
#define DISSET5 0xB6
#define PWCTR1 0xC0
#define PWCTR2 0xC1
#define PWCTR3 0xC2
#define PWCTR4 0xC3
#define PWCTR5 0xC4
#define VMCTR1 0xC5
#define RDID1 0xDA
#define RDID2 0xDB
#define RDID3 0xDC
#define RDID4 0xDD
#define PWCTR6 0xFC
#define GMCTRP1 0xE0
#define GMCTRN1 0xE1

//  ST7789 Orientation. From https://github.com/lupyuen/st7735-lcd-batch-rs/blob/master/src/lib.rs#L52-L58
#define Portrait 0x00
#define Landscape 0x60
#define PortraitSwapped 0xC0
#define LandscapeSwapped 0xA0

static int init_display(void);
static int set_window(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom);
static int hard_reset(void);
static int set_orientation(uint8_t orientation);
static int write_command(uint8_t command, const uint8_t *params, uint16_t len);
static int write_data(const uint8_t *data, uint16_t len);
static int transmit_spi(const uint8_t *data, uint16_t len);

/// Buffer for reading flash and writing to display
static uint8_t flash_buffer[COL_COUNT * BYTES_PER_PIXEL];

/// Display the image described by info to the ST7789 display controller using 2 colors. The first x lines (x = colorLine)
/// will be drawn in color1, the rest in color2.
int pinetime_display_image_colors(struct imgInfo* info, int posX, int posY, uint16_t color1, uint16_t color2, uint8_t colorLine) {
  int rc;
  int y = 0;
  uint16_t bufferIndex = 0;
  uint8_t isBackground = 1;
  const uint16_t backgroundColor = BLACK;
  uint16_t trueColor = backgroundColor;

  for (int i = 0; i < info->dataSize; i++) {
    uint8_t runLength = info->data[i];
    while (runLength) {
      flash_buffer[bufferIndex] = trueColor >> 8;
      flash_buffer[bufferIndex + 1] = trueColor & 0xff;
      bufferIndex += BYTES_PER_PIXEL;
      runLength -= 1;

      if (bufferIndex >= (info->width * BYTES_PER_PIXEL)) {
        rc = set_window(posX, y + posY, posX + info->width - 1, y + posY); assert(rc == 0);

        //  Write Pixels (RAMWR): st7735_lcd::draw() → set_pixel()
        rc = write_command(RAMWR, NULL, 0); assert(rc == 0);
        rc = write_data(flash_buffer, info->width * BYTES_PER_PIXEL); assert(rc == 0);
        bufferIndex = 0;
        y += 1;
      }
    }

    if (isBackground) {
      isBackground = 0;
      trueColor = (y < colorLine) ? color1 : color2;
    }
    else {
      isBackground = 1;
      trueColor = backgroundColor;
    }
    if(y >= info->height)
      break;
  }
  return 0;
}

/// Clear the display
void pinetime_clear_screen(void) {
  int rc = 0;
  for(int i = 0 ; i < COL_COUNT * BYTES_PER_PIXEL; i++) {
    flash_buffer[i] = 0;
  }
  for(int i = 0; i < ROW_COUNT; i++) {
    rc = set_window(0, i, COL_COUNT-1, i); assert(rc == 0);
    rc = write_command(RAMWR, NULL, 0); assert(rc == 0);
    rc = write_data(flash_buffer, COL_COUNT * BYTES_PER_PIXEL); assert(rc == 0);
  }
}

/// Display the image described by info at position (posX, posY) using default color (black and white)
int pinetime_display_image(struct imgInfo* info, int posX, int posY) {
  return pinetime_display_image_colors(info, posX, posY, WHITE, WHITE, 0);
}

/// Display the boot logo to ST7789 display controller
int pinetime_boot_display_image(void) {
  console_printf("Displaying boot logo...\n");  console_flush();

  int rc = init_display();  assert(rc == 0);
  rc = set_orientation(Landscape);  assert(rc == 0);
  pinetime_clear_screen();
  return pinetime_display_image(&bootLogoInfo, 0, 0);
}

/// Display the boot logo to ST7789 display controller using 2 colors. The first x lines (x = colorLine)
/// will be drawn in color1, the rest in color2.
int pinetime_boot_display_image_colors(uint16_t color1, uint16_t color2, uint8_t colorLine) {
  return pinetime_display_image_colors(&bootLogoInfo, 0, 0, color1, color2, colorLine);
}

/// Display the bootloader version to ST7789 display controller on the bottom of the display (centered)
int pinetime_version_image(void) {
  console_printf("Displaying version image...\n"); console_flush();
  return pinetime_display_image(&versionInfo, (COL_COUNT/2) - (versionInfo.width/2), ROW_COUNT - (versionInfo.height));
}

/// Set the ST7789 display window to the coordinates (left, top), (right, bottom)
static int set_window(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom) {
    assert(left < COL_COUNT && right < COL_COUNT && top < ROW_COUNT && bottom < ROW_COUNT);
    assert(left <= right);
    assert(top <= bottom);
    //  Set Address Window Columns (CASET): st7735_lcd::draw() → set_pixel() → set_address_window()
    int rc = write_command(CASET, NULL, 0); assert(rc == 0);
    uint8_t col_para[4] = { 0x00, left, 0x00, right };
    rc = write_data(col_para, 4); assert(rc == 0);

    //  Set Address Window Rows (RASET): st7735_lcd::draw() → set_pixel() → set_address_window()
    rc = write_command(RASET, NULL, 0); assert(rc == 0);
    uint8_t row_para[4] = { 0x00, top, 0x00, bottom };
    rc = write_data(row_para, 4); assert(rc == 0);
    return 0;
}

/// Runs commands to initialize the display. From https://github.com/lupyuen/st7735-lcd-batch-rs/blob/master/src/lib.rs
static int init_display(void) {
    //  Assume that SPI port 0 has been initialised by the SPI Flash Driver at startup.
    int rc;
    rc = hal_gpio_init_out(DISPLAY_RST, 1); assert(rc == 0);
    rc = hal_gpio_init_out(DISPLAY_CS, 1); assert(rc == 0);
    rc = hal_gpio_init_out(DISPLAY_DC, 0); assert(rc == 0);
    //  Switch on backlight
    rc = hal_gpio_init_out(DISPLAY_HIGH, 0); assert(rc == 0);

    hard_reset();
    write_command(SWRESET, NULL, 0);
    pinetime_delay_ms(200);
    write_command(SLPOUT, NULL, 0);
    pinetime_delay_ms(200);

    static const uint8_t FRMCTR1_PARA[] = { 0x01, 0x2C, 0x2D };
    write_command(FRMCTR1, FRMCTR1_PARA, sizeof(FRMCTR1_PARA));

    static const uint8_t FRMCTR2_PARA[] = { 0x01, 0x2C, 0x2D };
    write_command(FRMCTR2, FRMCTR2_PARA, sizeof(FRMCTR2_PARA));

    static const uint8_t FRMCTR3_PARA[] = { 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D };
    write_command(FRMCTR3, FRMCTR3_PARA, sizeof(FRMCTR3_PARA));

    static const uint8_t INVCTR_PARA[] = { 0x07 };
    write_command(INVCTR, INVCTR_PARA, sizeof(INVCTR_PARA));

    static const uint8_t PWCTR1_PARA[] = { 0xA2, 0x02, 0x84 };
    write_command(PWCTR1, PWCTR1_PARA, sizeof(PWCTR1_PARA));

    static const uint8_t PWCTR2_PARA[] = { 0xC5 };
    write_command(PWCTR2, PWCTR2_PARA, sizeof(PWCTR2_PARA));
    
    static const uint8_t PWCTR3_PARA[] = { 0x0A, 0x00 };
    write_command(PWCTR3, PWCTR3_PARA, sizeof(PWCTR3_PARA));
    
    static const uint8_t PWCTR4_PARA[] = { 0x8A, 0x2A };
    write_command(PWCTR4, PWCTR4_PARA, sizeof(PWCTR4_PARA));
    
    static const uint8_t PWCTR5_PARA[] = { 0x8A, 0xEE };
    write_command(PWCTR5, PWCTR5_PARA, sizeof(PWCTR5_PARA));
    
    static const uint8_t VMCTR1_PARA[] = { 0x0E };
    write_command(VMCTR1, VMCTR1_PARA, sizeof(VMCTR1_PARA));

    if (INVERTED) {
        write_command(INVON, NULL, 0);
    } else {
        write_command(INVOFF, NULL, 0);
    }
    if (RGB) {
        static const uint8_t MADCTL1_PARA[] = { 0x00 };
        write_command(MADCTL, MADCTL1_PARA, sizeof(MADCTL1_PARA));
    } else {
        static const uint8_t MADCTL2_PARA[] = { 0x08 };
        write_command(MADCTL, MADCTL2_PARA, sizeof(MADCTL2_PARA));
    }
    static const uint8_t COLMOD_PARA[] = { 0x05 };
    write_command(COLMOD, COLMOD_PARA, sizeof(COLMOD_PARA));

    write_command(DISPON, NULL, 0);
    pinetime_delay_ms(200);
    return 0;
}

/// Reset the display controller
static int hard_reset(void) {
    hal_gpio_write(DISPLAY_RST, 1);
    hal_gpio_write(DISPLAY_RST, 0);
    hal_gpio_write(DISPLAY_RST, 1);
    return 0;
}

/// Set the display orientation
static int set_orientation(uint8_t orientation) {
  int rc = 0;
    if (RGB) {
        uint8_t orientation_para[1] = { orientation };
        rc = write_command(MADCTL, orientation_para, 1);
        assert(rc == 0);
    } else {
        uint8_t orientation_para[1] = { orientation | 0x08 };
        rc = write_command(MADCTL, orientation_para, 1);
        assert(rc == 0);
    }
    return rc;
}

/// Transmit ST7789 command
static int write_command(uint8_t command, const uint8_t *params, uint16_t len) {
    hal_gpio_write(DISPLAY_DC, 0);
    int rc = transmit_spi(&command, 1);
    assert(rc == 0);
    if (rc == 0 && (params != NULL && len > 0)) {
        rc = write_data(params, len);
        assert(rc == 0);
    }
    return 0;
}

/// Transmit ST7789 data
static int write_data(const uint8_t *data, uint16_t len) {
    hal_gpio_write(DISPLAY_DC, 1);
    transmit_spi(data, len);
    return 0;
}

/// Write to the SPI port. From https://github.com/lupyuen/pinetime-rust-mynewt/blob/master/rust/mynewt/src/hal.rs
static int transmit_spi(const uint8_t *data, uint16_t len) {
    if (len == 0) { return 0; }
    //  Select the device
    hal_gpio_write(DISPLAY_CS, 0);
    //  Send the data
    int rc = hal_spi_txrx(DISPLAY_SPI,
        (void *) data,  //  TX Buffer
        NULL,  //  RX Buffer (don't receive)
        len);  //  Length
    assert(rc == 0);
    //  De-select the device
    hal_gpio_write(DISPLAY_CS, 1);
    return 0;
}

