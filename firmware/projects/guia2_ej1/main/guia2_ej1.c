/*! @mainpage Guia de proyecto numero 2
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_2  	|
 * |    TRIGGER     |   GPIO_3 		|
 * |     +5V        |    +5V        |
 * |      GND		|    GND        |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Document creation		                         |
 *
 * @author Florencia Gettig
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define CONFIG_BLINK_PERIOD_2 50

/*==================[internal data definition]===============================*/
bool start = false;
bool hold = false;
/*==================[internal functions declaration]=========================*/
void Task_measure(void *pvParameter)
{
	uint16_t distancia = 0;

	while (1)
	{
		if (start)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
			printf("Midiendo Distancia: %d", distancia);

			if (!hold)
			{
				LcdItsE0803Write(distancia);
				printf("not hold");
			}

			if (distancia < 10)
			{
				LedsOffAll();
			}
			else if (distancia < 20)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (distancia < 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
		else
		{
			LedsOffAll();
			LcdItsE0803Off();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

void Task_switches(void *pvParameter)
{
	uint8_t teclas;
	while (1)
	{
		teclas = SwitchesRead();

		switch (teclas)
		{
		case SWITCH_1:
			start = !start;
			break;
		case SWITCH_2:
			hold = !hold;
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_2 / portTICK_PERIOD_MS);
	}
	// Usar TEC1 para activar y detener la medición.
	// Usar TEC2 para mantener el resultado (“HOLD”).
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();

	xTaskCreate(&Task_measure, "Task_measure", 2048, NULL, 5, NULL);
	xTaskCreate(&Task_switches, "Task_switches", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/