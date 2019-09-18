/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/protocols/tfp/tfp.h"

#include "gas.h"

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_GET_VALUES: return get_values(message, response);
		case FID_GET_ADC_COUNT: return get_adc_count(message, response);
		case FID_SET_CALIBRATION: return set_calibration(message);
		case FID_GET_CALIBRATION: return get_calibration(message, response);
		case FID_SET_VALUES_CALLBACK_CONFIGURATION: return set_values_callback_configuration(message);
		case FID_GET_VALUES_CALLBACK_CONFIGURATION: return get_values_callback_configuration(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


BootloaderHandleMessageResponse get_values(const GetValues *data, GetValues_Response *response) {
	response->header.length     = sizeof(GetValues_Response);
	response->gas_type          = gas.type;
	response->humidity          = gas.humidity;
	response->temperature       = gas.temperature;
	response->gas_concentration = gas.ppb; // TODO: Round according to sensor

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_adc_count(const GetADCCount *data, GetADCCount_Response *response) {
	response->header.length = sizeof(GetADCCount_Response);
	response->adc_count     = gas.adc_count;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_calibration(const SetCalibration *data) {
	if(data->gas_type > GAS_GAS_TYPE_O3_NO2) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	gas.calibration_adc_count_zero         = data->adc_count_zero;
	gas.calibration_temperature_zero       = data->temperature_zero;
	gas.calibration_humidity_zero          = data->humidity_zero;
	gas.calibration_compensation_zero_low  = data->compensation_zero_low;
	gas.calibration_compensation_zero_high = data->compensation_zero_high;
	gas.calibration_ppm_span               = data->ppm_span;
	gas.calibration_adc_count_span         = data->adc_count_span;
	gas.calibration_temperature_span       = data->temperature_span;
	gas.calibration_humidity_span          = data->humidity_span;
	gas.calibration_compensation_span_low  = data->compensation_span_low;
	gas.calibration_compensation_span_high = data->compensation_span_high;
	gas.calibration_temperature_offset     = data->temperature_offset;
	gas.calibration_humidity_offset        = data->humidity_offset;
	gas.calibration_gas_type               = data->gas_type;
	gas.calibration_sensitivity            = data->sensitivity;

	gas.calibration_new                    = true;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_calibration(const GetCalibration *data, GetCalibration_Response *response) {
	response->header.length          = sizeof(GetCalibration_Response);
	response->adc_count_zero         = gas.calibration_adc_count_zero;
	response->temperature_zero       = gas.calibration_temperature_zero;
	response->humidity_zero          = gas.calibration_humidity_zero;
	response->compensation_zero_low  = gas.calibration_compensation_zero_low;
	response->compensation_zero_high = gas.calibration_compensation_zero_high;
	response->ppm_span               = gas.calibration_ppm_span;
	response->adc_count_span         = gas.calibration_adc_count_span;
	response->temperature_span       = gas.calibration_temperature_span;
	response->humidity_span          = gas.calibration_humidity_span;
	response->compensation_span_low  = gas.calibration_compensation_span_low;
	response->compensation_span_high = gas.calibration_compensation_span_high;
	response->temperature_offset     = gas.calibration_temperature_offset;
	response->humidity_offset        = gas.calibration_humidity_offset;
	response->gas_type               = gas.calibration_gas_type;
	response->sensitivity            = gas.calibration_sensitivity;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_values_callback_configuration(const SetValuesCallbackConfiguration *data) {
	gas.period              = data->period;
	gas.value_has_to_change = data->value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_values_callback_configuration(const GetValuesCallbackConfiguration *data, GetValuesCallbackConfiguration_Response *response) {
	response->header.length       = sizeof(GetValuesCallbackConfiguration_Response);
	response->period              = gas.period;
	response->value_has_to_change = gas.value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}


bool handle_values_callback(void) {
	static bool is_buffered = false;
	static Values_Callback cb;

	static uint32_t last_time              = 0;
	static int32_t  last_gas_concentration = 0;
	static int16_t  last_temperature       = 0;
	static uint16_t last_humidity          = 0;
	static uint8_t  last_gas_type          = 0;

	if(!is_buffered) {
		if(gas.period == 0 || !system_timer_is_time_elapsed_ms(last_time, gas.period)) {
			return false;
		}

		if(gas.ppb         == last_gas_concentration && // TODO: calculate per sensor type
		   gas.temperature == last_temperature &&
		   gas.humidity    == last_humidity &&
		   gas.type        == last_gas_type) {
			return false;
		}

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(Values_Callback), FID_CALLBACK_VALUES);
		cb.gas_concentration   = gas.ppb; // TODO: calculate per sensor type
		cb.gas_type            = gas.type;
		cb.humidity            = gas.humidity;
		cb.temperature         = gas.temperature;

		last_gas_concentration = cb.gas_concentration;
		last_gas_type          = cb.gas_type;
		last_humidity          = cb.humidity;
		last_temperature       = cb.temperature;

		last_time              = system_timer_get_ms();
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(Values_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
