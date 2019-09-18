/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * hdc1080.h: Driver for HDC1080 Humidity/Temperature sensor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef HDC1080_H
#define HDC1080_H

void hdc1080_task_tick(void);
void hdc1080_task_init(void);

#define HDC1080_REG_TEMPERATURE     0x00
#define HDC1080_REG_HUMIDITY        0x01
#define HDC1080_REG_CONFIGURATION   0x02
#define HDC1080_REG_SERIAL_ID_LOW   0xFB
#define HDC1080_REG_SERIAL_ID_MID   0xFC
#define HDC1080_REG_SERIAL_ID_HIGH  0xFD
#define HDC1080_REG_MANUFACTURER_ID 0xFE
#define HDC1080_REG_DEVICE_ID       0xFF

#define HDC1080_RESOLUTION_T_14BIT  0b00
#define HDC1080_RESOLUTION_T_11BIT  0b01

#define HDC1080_RESOLUTION_H_14BIT  0b00
#define HDC1080_RESOLUTION_H_11BIT  0b01
#define HDC1080_RESOLUTION_H_8BIT   0b10

#define HDC1080_MANUFACTURER_ID     0x5449
#define HDC1080_DEVICE_ID           0x1050

#define HDC1080_POWERUP_TIME        25 // 15ms until we can read humidity/temperature
#define HDC1080_CONVERSION_TIME     20 // 7ms for conversion, see page 5 "Electrical Characteristics"

#endif