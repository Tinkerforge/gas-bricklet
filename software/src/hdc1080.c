/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * hdc1080.c: Driver for HDC1080 Humidity/Temperature sensor
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

#include "hdc1080.h"

#include "configs/config_hdc1080.h"

#include "bricklib2/os/coop_task.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "gas.h"

#define HDC1080_TIME_BETWEEN_MEASUREMENTS 1000 // in ms

void hdc1080_task_tick(void) {
	static uint32_t last_time = 0;
	if(system_timer_is_time_elapsed_ms(last_time, HDC1080_TIME_BETWEEN_MEASUREMENTS)) {
		uint8_t data[4];
		gas_task_write_register(HDC1080_I2C_ADDRESS, HDC1080_REG_TEMPERATURE, 0, (uint8_t*)data, true);
		coop_task_sleep_ms(HDC1080_CONVERSION_TIME);
		gas_task_read_direct(HDC1080_I2C_ADDRESS, 4, data, false);

		gas.temperature = ((int32_t)(data[1] | (data[0] << 8)))*16500/(1 << 16) - 4000;
		gas.humidity    = ((int32_t)(data[3] | (data[2] << 8)))*10000/(1 << 16);
		logd("HDC1080: Temperature %d, Humidity %d\n\r", gas.temperature, gas.humidity);

		last_time = system_timer_get_ms();
	}
}

void hdc1080_task_init(void) {
	// Use 14 bit, enable temperature and humidity
	uint8_t config[2] = {0b00010000, 0b00000000};
	gas_task_write_register(HDC1080_I2C_ADDRESS, HDC1080_REG_CONFIGURATION, 2, config, true);
}