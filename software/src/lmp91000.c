/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * lmp91000.c: Driver for LMP91000 Potentiostat
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

#include "lmp91000.h"

#include "configs/config_lmp91000.h"

#include "bricklib2/os/coop_task.h"
#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "gas.h"

#define LMP91000_TIME_BETWEEN_MEASUREMENTS 1000 // in ms

// {TIACN, REFCN, MODECN}
const uint8_t lmp91000_configuration[][3] = {
	{3,  145, 3}, // CO
	{27, 147, 3}, // EtOH
	{3,  144, 3}, // H2S
	{3,  182, 3}, // SO2
	{3,  166, 3}, // NO2
	{3,  161, 3}, // O3
	{3,  149, 3}, // IAQ
	{3,  166, 3}, // RESP
	{3,  161, 3}, // O3/NO2
};

void lmp91000_task_tick(void) {
#if LOGGING_LEVEL == LOGGING_DEBUG
	static uint32_t last_time = 0;
	if(system_timer_is_time_elapsed_ms(last_time, LMP91000_TIME_BETWEEN_MEASUREMENTS)) {
		const uint8_t regs[5] = {LMP91000_REG_LOCK, LMP91000_REG_MODECN, LMP91000_REG_REFCN, LMP91000_REG_STATUS, LMP91000_REG_TIACN};

		for(uint8_t i = 0; i < 5; i++) {
			uint8_t data = 0;
			gas_task_read_register(LMP91000_I2C_ADDRESS, regs[i], 1, &data);
			logd("LMP91000: Register %x -> %x\n\r", regs[i], data);
		}

		last_time = system_timer_get_ms();
	}
#endif
}

void lmp91000_task_init(void) {
	uint8_t unlock = 0;
	gas_task_write_register(LMP91000_I2C_ADDRESS, LMP91000_REG_LOCK,   1, &unlock,                              true);

	gas_task_write_register(LMP91000_I2C_ADDRESS, LMP91000_REG_TIACN,  1, &lmp91000_configuration[gas.type][0], true);
	gas_task_write_register(LMP91000_I2C_ADDRESS, LMP91000_REG_REFCN,  1, &lmp91000_configuration[gas.type][1], true);
	gas_task_write_register(LMP91000_I2C_ADDRESS, LMP91000_REG_MODECN, 1, &lmp91000_configuration[gas.type][2], true);

	gas.tia_gain = (lmp91000_configuration[gas.type][0] & 0b00011100) >> 2;
}