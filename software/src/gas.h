/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * gas.h: Driver for general Gas type and Concentration detection
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

#ifndef GAS_H
#define GAS_H

#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"

typedef struct {
	I2CFifo i2c_fifo;
	bool i2c_mutex;

	uint8_t type;
	int32_t na_per_ppm;

	int16_t temperature;
	uint16_t humidity;

	int16_t temperature_offset;
	int16_t humidity_offset;

	int32_t adc_count;
	int32_t adc_count_zero;

	uint8_t tia_gain;

	double ppb;


	uint32_t period;
	bool value_has_to_change;

	uint32_t calibration_adc_count_zero;
	int16_t  calibration_temperature_zero;
	int16_t  calibration_humidity_zero;
	int32_t  calibration_compensation_zero_low;
	int32_t  calibration_compensation_zero_high;
	uint32_t calibration_ppm_span;
	uint32_t calibration_adc_count_span;
	int16_t  calibration_temperature_span;
	int16_t  calibration_humidity_span;
	int32_t  calibration_compensation_span_low;
	int32_t  calibration_compensation_span_high;
	int16_t  calibration_temperature_offset;
	int16_t  calibration_humidity_offset;
	uint8_t  calibration_gas_type;
	int32_t  calibration_sensitivity;

	bool     calibration_new;
} Gas;

extern Gas gas;

uint32_t gas_task_read_register(const uint8_t address, const I2C_FIFO_REG_TYPE reg, const uint32_t length, uint8_t *data);
uint32_t gas_task_write_register(const uint8_t address, const I2C_FIFO_REG_TYPE reg, const uint32_t length, const uint8_t *data, const bool send_stop);
uint32_t gas_task_read_direct(const uint8_t address, const uint32_t length, uint8_t *data, const bool restart);
uint32_t gas_task_write_direct(const uint8_t address, const uint32_t length, const uint8_t *data, const bool send_stop);

void gas_init(void);
void gas_tick(void);

#endif
