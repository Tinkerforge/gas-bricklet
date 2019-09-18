/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * gas.c: Driver for general Gas type and Concentration detection
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

#include "gas.h"

#include "configs/config_gas.h"
#include "configs/config_lmp91000.h"
#include "configs/config_mcp3423.h"
#include "configs/config_hdc1080.h"

#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"
#include "bricklib2/os/coop_task.h"

#include "communication.h"

#include "lmp91000.h"
#include "hdc1080.h"
#include "mcp3423.h"

#define GAS_CALIBRATION_PAGE           1
#define GAS_CALIBRATION_MAGIC_POS      0
#define GAS_CALIBRATION_DATA_POS       1 // 1 to 14
#define GAS_CALIBRATION_CHECKSUM_POS   15
#define GAS_CALIBRATION_MAGIC          0x12345678

#define GAS_TIME_BETWEEN_INIT_AND_TICK 300 // in ms

#define GAS_ADC_18BIT_MAX              262143
#define GAS_TEMPERATURE_THRESHOLD_ZERO 25.0
#define GAS_TEMPERATURE_THRESHOLD_SPAN 25.0

const uint32_t gas_tiagain_to_rgain[8] = {
	499000, 2735, 3476, 6903, 13618, 32706, 96737, 205713
};

// {Zero Low, Zero High, Span Low, Span High}
const double gas_sensor_compensation[][4] = {
	{0.0,   2.375, 0.6, 0.4}, // CO
	{1.7,  22.5,   1.2, 0.0}, // EtOH
	{0.0,   0.0,   0.3, 0.0}, // H2S
	{0.0,  15.0,   1.2, 0.5}, // SO2
	{0.0, - 1.6,   0.0, 0.4}, // NO2
	{0.0,   0.0,   0.0, 0.5}, // O3
	{0.0,   0.0,   0.0, 0.0}, // IAQ
	{0.0,   0.0,   0.0, 0.0}, // RESP
	{0.0,   0.0,   0.0, 0.0}, // O3/NO
};


CoopTask gas_task;
Gas gas;

uint32_t gas_task_read_register(const uint8_t address, const I2C_FIFO_REG_TYPE reg, const uint32_t length, uint8_t *data) {
	while(gas.i2c_mutex) {
		coop_task_yield();
	}

	gas.i2c_mutex = true;
	gas.i2c_fifo.address = address;

	uint32_t ret = i2c_fifo_coop_read_register(&gas.i2c_fifo, reg, length, data);

	gas.i2c_mutex = false;

	return ret;
}

uint32_t gas_task_write_register(const uint8_t address, const I2C_FIFO_REG_TYPE reg, const uint32_t length, const uint8_t *data, const bool send_stop) {
	while(gas.i2c_mutex) {
		coop_task_yield();
	}

	gas.i2c_mutex = true;
	gas.i2c_fifo.address = address;

	uint32_t ret = i2c_fifo_coop_write_register(&gas.i2c_fifo, reg, length, data, send_stop);

	gas.i2c_mutex = false;

	return ret;
}

uint32_t gas_task_read_direct(const uint8_t address, const uint32_t length, uint8_t *data, const bool restart) {
	while(gas.i2c_mutex) {
		coop_task_yield();
	}

	gas.i2c_mutex = true;
	gas.i2c_fifo.address = address;

	uint32_t ret = i2c_fifo_coop_read_direct(&gas.i2c_fifo, length, data, restart);

	gas.i2c_mutex = false;

	return ret;
}

uint32_t gas_task_write_direct(const uint8_t address, const uint32_t length, const uint8_t *data, const bool send_stop) {
	while(gas.i2c_mutex) {
		coop_task_yield();
	}

	gas.i2c_mutex = true;
	gas.i2c_fifo.address = address;

	uint32_t ret = i2c_fifo_coop_write_direct(&gas.i2c_fifo, length, data, send_stop);

	gas.i2c_mutex = false;

	return ret;
}

void gas_calibration_read(void) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];

	bootloader_read_eeprom_page(GAS_CALIBRATION_PAGE, page);
	uint32_t checksum = 0;
	for(uint8_t i = 0; i < GAS_CALIBRATION_CHECKSUM_POS; i++) {
		checksum = checksum ^ page[i];
	}

	if(page[GAS_CALIBRATION_MAGIC_POS] != GAS_CALIBRATION_MAGIC) {
		logd("Calibration Read: Wrong magic %x != %x\n\r", page[GAS_CALIBRATION_MAGIC_POS], GAS_CALIBRATION_MAGIC);
		return;
	}

	if(page[GAS_CALIBRATION_CHECKSUM_POS] != checksum) {
		logd("Calibration Read: Wrong checksum %x != %x\n\r", page[GAS_CALIBRATION_CHECKSUM_POS], checksum);
		return;
	}

	gas.calibration_adc_count_zero         = page[GAS_CALIBRATION_DATA_POS +  0];
	gas.calibration_temperature_zero       = page[GAS_CALIBRATION_DATA_POS +  1];
	gas.calibration_humidity_zero          = page[GAS_CALIBRATION_DATA_POS +  2];
	gas.calibration_compensation_zero_low  = page[GAS_CALIBRATION_DATA_POS +  3];
	gas.calibration_compensation_zero_high = page[GAS_CALIBRATION_DATA_POS +  4];
	gas.calibration_ppm_span               = page[GAS_CALIBRATION_DATA_POS +  5];
	gas.calibration_adc_count_span         = page[GAS_CALIBRATION_DATA_POS +  6];
	gas.calibration_temperature_span       = page[GAS_CALIBRATION_DATA_POS +  7];
	gas.calibration_humidity_span          = page[GAS_CALIBRATION_DATA_POS +  8];
	gas.calibration_compensation_span_low  = page[GAS_CALIBRATION_DATA_POS +  9];
	gas.calibration_compensation_span_high = page[GAS_CALIBRATION_DATA_POS + 10];
	gas.calibration_temperature_offset     = page[GAS_CALIBRATION_DATA_POS + 11];
	gas.calibration_humidity_offset        = page[GAS_CALIBRATION_DATA_POS + 12];
	gas.calibration_sensitivity            = page[GAS_CALIBRATION_DATA_POS + 13]; 


	// Use GAS_CALIBRATION_MAGIC here to differentiate between new
	// versions of calibration. Currently we only use three values here.
	gas.adc_count_zero     = gas.calibration_adc_count_zero;
	gas.na_per_ppm         = gas.calibration_sensitivity;
	gas.temperature_offset = gas.calibration_temperature_offset;
	gas.humidity_offset    = gas.calibration_temperature_offset;
}

void gas_calibration_write(void) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];

	page[GAS_CALIBRATION_MAGIC_POS    ] = GAS_CALIBRATION_MAGIC;

	page[GAS_CALIBRATION_DATA_POS +  0] = gas.calibration_adc_count_zero;
	page[GAS_CALIBRATION_DATA_POS +  1] = gas.calibration_temperature_zero;
	page[GAS_CALIBRATION_DATA_POS +  2] = gas.calibration_humidity_zero;
	page[GAS_CALIBRATION_DATA_POS +  3] = gas.calibration_compensation_zero_low;
	page[GAS_CALIBRATION_DATA_POS +  4] = gas.calibration_compensation_zero_high;
	page[GAS_CALIBRATION_DATA_POS +  5] = gas.calibration_ppm_span;
	page[GAS_CALIBRATION_DATA_POS +  6] = gas.calibration_adc_count_span;
	page[GAS_CALIBRATION_DATA_POS +  7] = gas.calibration_temperature_span;
	page[GAS_CALIBRATION_DATA_POS +  8] = gas.calibration_humidity_span;
	page[GAS_CALIBRATION_DATA_POS +  9] = gas.calibration_compensation_span_low;
	page[GAS_CALIBRATION_DATA_POS + 10] = gas.calibration_compensation_span_high;
	page[GAS_CALIBRATION_DATA_POS + 11] = gas.calibration_temperature_offset;
	page[GAS_CALIBRATION_DATA_POS + 12] = gas.calibration_humidity_offset;
	page[GAS_CALIBRATION_DATA_POS + 13] = gas.calibration_sensitivity;

	uint32_t checksum = 0;
	for(uint8_t i = 0; i < GAS_CALIBRATION_CHECKSUM_POS; i++) {
		checksum = checksum ^ page[i];
	}

	page[GAS_CALIBRATION_CHECKSUM_POS] = checksum;

	bootloader_write_eeprom_page(GAS_CALIBRATION_PAGE, page);
}

void gas_calculate_ppb(void) {
	const double rgain            = gas_tiagain_to_rgain[gas.tia_gain];
	const double zero_low         = gas_sensor_compensation[gas.type][0];
	const double zero_high        = gas_sensor_compensation[gas.type][1];
	const double span_low         = gas_sensor_compensation[gas.type][2];
	const double span_high        = gas_sensor_compensation[gas.type][3];
	const double temperature      = gas.temperature/100.0;

	const double zero             = temperature < GAS_TEMPERATURE_THRESHOLD_ZERO ? zero_low : zero_high;
	const double span             = temperature < GAS_TEMPERATURE_THRESHOLD_SPAN ? span_low : span_high;

	const double na               = ((double)(gas.adc_count - gas.adc_count_zero))/GAS_ADC_18BIT_MAX * 2.048/rgain * 1E9;
	const double zero_compensated = na -  (zero *        (temperature - GAS_TEMPERATURE_THRESHOLD_ZERO));
	const double span_compensated = 1.0 - (span/1000.0 * (temperature - GAS_TEMPERATURE_THRESHOLD_SPAN));

	gas.ppb = zero_compensated * span_compensated / ((double)gas.na_per_ppm) * 1E5;
	logd("Gas: PPT %d, PPB %d, PPM %d\n\r", (int)(gas.ppb*1000), (int)gas.ppb, (int)(gas.ppb/1000.0));
}


void gas_task_tick(void) {
	coop_task_sleep_ms(HDC1080_POWERUP_TIME);

	lmp91000_task_init();
	hdc1080_task_init();
	mcp3423_task_init();

	coop_task_sleep_ms(GAS_TIME_BETWEEN_INIT_AND_TICK);

	uint32_t last_adc_count = 0;
	while(true) {
		lmp91000_task_tick();
		hdc1080_task_tick();
		mcp3423_task_tick();

		if(gas.calibration_new) {
			gas.calibration_new = false;
			gas_calibration_write();

			// Read calibration again to use newly set values
			// depending on different MAGIC numbers
			gas_calibration_read();
		}

		if(last_adc_count != gas.adc_count) {
			last_adc_count = gas.adc_count;
			gas_calculate_ppb();
		}

		coop_task_yield();
	}
}

void gas_init_i2c(void) {
	gas.i2c_fifo.baudrate         = GAS_I2C_BAUDRATE;
	gas.i2c_fifo.address          = 0; // set by read/write method
	gas.i2c_fifo.i2c              = GAS_I2C;

	gas.i2c_fifo.scl_port         = GAS_SCL_PORT;
	gas.i2c_fifo.scl_pin          = GAS_SCL_PIN;
	gas.i2c_fifo.scl_mode         = GAS_SCL_PIN_MODE;
	gas.i2c_fifo.scl_input        = GAS_SCL_INPUT;
	gas.i2c_fifo.scl_source       = GAS_SCL_SOURCE;
	gas.i2c_fifo.scl_fifo_size    = GAS_SCL_FIFO_SIZE;
	gas.i2c_fifo.scl_fifo_pointer = GAS_SCL_FIFO_POINTER;

	gas.i2c_fifo.sda_port         = GAS_SDA_PORT;
	gas.i2c_fifo.sda_pin          = GAS_SDA_PIN;
	gas.i2c_fifo.sda_mode         = GAS_SDA_PIN_MODE;
	gas.i2c_fifo.sda_input        = GAS_SDA_INPUT;
	gas.i2c_fifo.sda_source       = GAS_SDA_SOURCE;
	gas.i2c_fifo.sda_fifo_size    = GAS_SDA_FIFO_SIZE;
	gas.i2c_fifo.sda_fifo_pointer = GAS_SDA_FIFO_POINTER;

	i2c_fifo_init(&gas.i2c_fifo);
}

void gas_init(void) {
	memset(&gas, 0, sizeof(Gas));

	gas_calibration_read();

	// TODO: Test-Calibration:
	// gas.adc_count_zero = 107292;
	// gas.na_per_ppm     = 290;

	gas_init_i2c();
	
	coop_task_init(&gas_task, gas_task_tick);
}

void gas_tick(void) {
	coop_task_tick(&gas_task);
}