/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants

#define GAS_THRESHOLD_OPTION_OFF 'x'
#define GAS_THRESHOLD_OPTION_OUTSIDE 'o'
#define GAS_THRESHOLD_OPTION_INSIDE 'i'
#define GAS_THRESHOLD_OPTION_SMALLER '<'
#define GAS_THRESHOLD_OPTION_GREATER '>'

#define GAS_GAS_TYPE_CO 0
#define GAS_GAS_TYPE_ETOH 1
#define GAS_GAS_TYPE_H2S 2
#define GAS_GAS_TYPE_SO2 3
#define GAS_GAS_TYPE_NO2 4
#define GAS_GAS_TYPE_O3 5
#define GAS_GAS_TYPE_IAQ 6
#define GAS_GAS_TYPE_RESP 7
#define GAS_GAS_TYPE_O3_NO2 8

#define GAS_BOOTLOADER_MODE_BOOTLOADER 0
#define GAS_BOOTLOADER_MODE_FIRMWARE 1
#define GAS_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define GAS_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define GAS_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define GAS_BOOTLOADER_STATUS_OK 0
#define GAS_BOOTLOADER_STATUS_INVALID_MODE 1
#define GAS_BOOTLOADER_STATUS_NO_CHANGE 2
#define GAS_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define GAS_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define GAS_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define GAS_STATUS_LED_CONFIG_OFF 0
#define GAS_STATUS_LED_CONFIG_ON 1
#define GAS_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define GAS_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_GET_VALUES 1
#define FID_GET_ADC_COUNT 2
#define FID_SET_CALIBRATION 3
#define FID_GET_CALIBRATION 4
#define FID_SET_VALUES_CALLBACK_CONFIGURATION 5
#define FID_GET_VALUES_CALLBACK_CONFIGURATION 6

#define FID_CALLBACK_VALUES 7

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetValues;

typedef struct {
	TFPMessageHeader header;
	int32_t gas_concentration;
	int16_t temperature;
	uint16_t humidity;
	uint8_t gas_type;
} __attribute__((__packed__)) GetValues_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetADCCount;

typedef struct {
	TFPMessageHeader header;
	uint32_t adc_count;
} __attribute__((__packed__)) GetADCCount_Response;

typedef struct {
	TFPMessageHeader header;
	uint32_t adc_count_zero;
	int16_t temperature_zero;
	int16_t humidity_zero;
	int32_t compensation_zero_low;
	int32_t compensation_zero_high;
	uint32_t ppm_span;
	uint32_t adc_count_span;
	int16_t temperature_span;
	int16_t humidity_span;
	int32_t compensation_span_low;
	int32_t compensation_span_high;
	uint8_t gas_type;
	int32_t sensitivity;
} __attribute__((__packed__)) SetCalibration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetCalibration;

typedef struct {
	TFPMessageHeader header;
	uint32_t adc_count_zero;
	int16_t temperature_zero;
	int16_t humidity_zero;
	int32_t compensation_zero_low;
	int32_t compensation_zero_high;
	uint32_t ppm_span;
	uint32_t adc_count_span;
	int16_t temperature_span;
	int16_t humidity_span;
	int32_t compensation_span_low;
	int32_t compensation_span_high;
	uint8_t gas_type;
	int32_t sensitivity;
} __attribute__((__packed__)) GetCalibration_Response;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) SetValuesCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetValuesCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) GetValuesCallbackConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	int32_t gas_concentration;
	int16_t temperature;
	uint16_t humidity;
	uint8_t gas_type;
} __attribute__((__packed__)) Values_Callback;


// Function prototypes
BootloaderHandleMessageResponse get_values(const GetValues *data, GetValues_Response *response);
BootloaderHandleMessageResponse get_adc_count(const GetADCCount *data, GetADCCount_Response *response);
BootloaderHandleMessageResponse set_calibration(const SetCalibration *data);
BootloaderHandleMessageResponse get_calibration(const GetCalibration *data, GetCalibration_Response *response);
BootloaderHandleMessageResponse set_values_callback_configuration(const SetValuesCallbackConfiguration *data);
BootloaderHandleMessageResponse get_values_callback_configuration(const GetValuesCallbackConfiguration *data, GetValuesCallbackConfiguration_Response *response);

// Callbacks
bool handle_values_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 1
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_values_callback, \


#endif
