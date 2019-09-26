/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * mcp3423.c: Driver for MCP3423 ADC
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

#include "mcp3423.h"

#include "configs/config_mcp3423.h"

#include "bricklib2/os/coop_task.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "gas.h"

#define MCP3423_MAX_VALUE ((1 << 18)-1)

// 4 SPS
#define MCP3423_TIME_BETWEEN_MEASUREMENTS 250 // in ms

void mcp3423_task_tick(void) {
	static uint32_t last_time = 0;
	if(system_timer_is_time_elapsed_ms(last_time, MCP3423_TIME_BETWEEN_MEASUREMENTS)) {
		uint8_t data[4];

		gas_task_read_direct(MCP3423_I2C_ADDRESS, 4, data, false);
		logd("MCP3423: Raw %x %x %x %x\n\r", data[0], data[1], data[2], data[3]);

		if(!(data[3] & MCP3423_CONF_MSK_RDY1)) {
			// 18 bits
			gas.adc_count = MCP3423_MAX_VALUE - ((data[2] | (data[1] << 8) | ((data[0] & 0x03) << 16)));

			logd("MCP3423: ADC Count %d\n\r", gas.adc_count);
		}

		last_time = system_timer_get_ms();
	}
}

void mcp3423_task_init(void) {
	// Configure ch0, gain 1x and 4 SPS
	uint8_t configuration = MCP3423_CONF_MSK_Gx1 | MCP3423_CONF_MSK_CH0 | MCP3423_CONF_MSK_MODE_CONT | MCP3423_CONF_MSK_SPS4 | MCP3423_CONF_MSK_RDY0;
	gas_task_write_direct(MCP3423_I2C_ADDRESS, 1, &configuration, true);
}