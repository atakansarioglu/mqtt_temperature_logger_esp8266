/**
 * @file      main.c
 * @author    Atakan S.
 * @date      01/06/2018
 * @version   1.0
 * @brief     PAHO MQTT Embedded demo that blinks LED over MQTT.
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

// Include
#include "FreeRTOS.h"
#include "task.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "networkwrapper.h"
#include "temperature.h"
#include "led.h"
#include <stm32f10x.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Settings.
#define CONNECTION_KEEPALIVE_S 60UL

/*
 * @brief Publishes temperature value to mqtt.
 * @param pvParameters Task call parameters.
 */
static void prvTemperaturePublisher(void *pvParameters) {
	unsigned char buffer[128];
	MQTTTransport transporter;
	int result;
	int length;

	// Transport layer uses the esp8266 networkwrapper.
	static transport_iofunctions_t iof = {network_send, network_recv};
	int transport_socket = transport_open(&iof);

	// State machine.
	int internalState = 0;
	while(true) {
		switch(internalState){
		case 0:	{
			// Turn the LED off.
			vLedWrite(0, false);

			// Initialize the temperature sensor.
			temperature_init();

			// Initialize the network and connect to
			network_init();
			if(network_connect("iot.eclipse.org", 1883, CONNECTION_KEEPALIVE_S, false) == 0){
				// To the next state.
				internalState++;
			}
		} break;
		case 1:	{
			// Populate the transporter.
			transporter.sck = &transport_socket;
			transporter.getfn = transport_getdatanb;
			transporter.state = 0;

			// Populate the connect struct.
			MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
			connectData.MQTTVersion = 3;
			connectData.clientID.cstring = "TemperaturePublisher";
			connectData.keepAliveInterval = CONNECTION_KEEPALIVE_S * 2;
			length = MQTTSerialize_connect(buffer, sizeof(buffer), &connectData);

			// Send CONNECT to the mqtt broker.
			if((result = transport_sendPacketBuffer(transport_socket, buffer, length)) == length){
				// To the next state.
				internalState++;
			} else {
				// Start over.
				internalState = 0;
			}
		} break;
		case 2:	{
			// Wait for CONNACK response from the mqtt broker.
			while(true) {
				// Wait until the transfer is done.
				if((result = MQTTPacket_readnb(buffer, sizeof(buffer), &transporter)) == CONNACK){
					// Check if the connection was accepted.
					unsigned char sessionPresent, connack_rc;
					if ((MQTTDeserialize_connack(&sessionPresent, &connack_rc, buffer, sizeof(buffer)) != 1) || (connack_rc != 0)){
						// Start over.
						internalState = 0;
						break;
					}else{
						// To the next state.
						internalState++;
						break;
					}
				} else if (result == -1) {
					// Start over.
					internalState = 0;
					break;
				}
			}
		} break;
		case 3:	{
			// Turn the LED on.
			vLedWrite(0, true);

			// Set delay timer.
			TickType_t wakeTime = xTaskGetTickCount();

			// Populate the publish message.
			MQTTString topicString = MQTTString_initializer;
			topicString.cstring = "temperature/value";
			unsigned char payload[16];
			length = MQTTSerialize_publish(buffer, sizeof(buffer), 0, 0, 0, 0, topicString, payload, (length = sprintf(payload, "%d", (int)temperature_read())));

			// Send PUBLISH to the mqtt broker.
			if((result = transport_sendPacketBuffer(transport_socket, buffer, length)) == length){
				// Turn the LED off.
				vLedWrite(0, false);

				// Wait 5s.
				vTaskDelayUntil(&wakeTime, pdMS_TO_TICKS(5000));
			} else {
				// Start over.
				internalState = 0;
			}
		} break;
		default:
			internalState = 0;
		}
	}
}

/*
 * @brief Time provider for the networkwrapper..
 * @return Time in ms.
 */
long unsigned int network_gettime_ms(void) {
	return (xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/*
 * @brief Main.
 * @return Non-zero on error.
 */
int main(void) {
	// Create the task.
	xTaskCreate(prvTemperaturePublisher, "TempPub", 512UL, NULL, (tskIDLE_PRIORITY + 1), NULL);

	// Start FreeRTOS.
	vTaskStartScheduler();

	// Program should not reach here.
	return 1;
}
