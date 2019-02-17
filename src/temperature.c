/**
 * @file      temperature.c
 * @author    Atakan S.
 * @date      01/06/2018
 * @version   1.0
 * @brief     Simply reads temperature on stm32f10x.
 *
 * @copyright Copyright (c) 2018 Atakan SARIOGLU ~ www.atakansarioglu.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

// Include.
#include "temperature.h"
#include <stm32f10x.h>

// Settings.
#define TEMPERATURE_VCC_MV 3300UL
#define TEMPERATURE_V25_MV 1410UL
#define TEMPERATURE_AVGSLOPE_MV 4.3f
#define TEMPERATURE_FULLSCALE 4096UL

/*
 * @brief Initializes the adc for temperature reading.
 */
void temperature_init(void) {
	ADC_InitTypeDef ADC_InitStructure;
	
	// Setup ADC clock.
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	// Configure.
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	// Configure the temperature sensor channel.
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_239Cycles5);

	// Enable.
	ADC_Cmd(ADC1, ENABLE);

	//ADC calibration procedure.
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

/*
 * @brief Initializes the adc for temperature reading.
 * @return Temperature value in celcius.
 */
float temperature_read(void) {
	// Start the conversion.
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	uint16_t adc_raw = ADC_GetConversionValue(ADC1);
	
	// Convert to celcius.
	float adc_celcius = ((((float)TEMPERATURE_V25_MV * TEMPERATURE_FULLSCALE / TEMPERATURE_VCC_MV) - adc_raw) / TEMPERATURE_AVGSLOPE_MV) + 25;
	
	// Return.
	return adc_celcius;
}
