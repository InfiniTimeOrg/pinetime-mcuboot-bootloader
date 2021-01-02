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
//  Render boot graphic and check for manual rollback
#ifndef __PINETIME_BOOT_H__
#define __PINETIME_BOOT_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {  //  Expose the types and functions below to C functions.
#endif

//  Colors
#define BLACK 0
#define WHITE 0xffff
#define RED 0xF800
#define BLUE 0x001F
#define GREEN 0x07E0

/// Init the display and render the boot graphic. Called by sysinit() during startup, defined in pkg.yml.
void pinetime_boot_init(void);

/// Write a converted graphic file to SPI Flash
int pinetime_boot_write_image(void);

/// Display the boot logo to ST7789 display controller
int pinetime_boot_display_image(void);

/// Display the boot logo to ST7789 display controller using 2 colors. The first x lines (x = colorLine)
/// will be drawn in color1, the rest in color2.
int pinetime_boot_display_image_colors(uint16_t color1, uint16_t color2, uint8_t colorLine);

/// Display the bootloader version to ST7789 display controller
int pinetime_version_image(void);

/// Clear the display
void pinetime_clear_screen(void);

/// Check whether the watch button is pressed
void pinetime_boot_check_button(void);



#ifdef __cplusplus
}
#endif

#endif  //  __PINETIME_BOOT_H__
