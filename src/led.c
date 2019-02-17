/**
 * @file      led.c
 * @author    Atakan S.
 * @date      01/06/2018
 * @version   1.0
 * @brief     Simply controls the LED on stm32f103 Nucleo board.
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
#include "led.h"
#include <stm32f10x.h>
#include <assert.h>

// Settings.
#define LED_COUNT 1

/*
 * @brief Controls various LEDs.
 * @param ledID The id number of the LED to modify.
 * @param ledON If true, the LED is on, otherwise off.
 */
void vLedWrite(const unsigned char ledID, const unsigned char ledON) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// Check the id.
	assert(ledID < LED_COUNT);
	
	// Pick the LED.
	switch(ledID){
	case 0:
		// Enable peripheral clocks.
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

		// Configure PC9 output pushpull mode.
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		// Set state.
		GPIO_WriteBit(GPIOA, GPIO_Pin_5, ledON);
		break;
	}
}
