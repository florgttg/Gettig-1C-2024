/*! @mainpage Guia de proyecto numero 2, actividad 1
 *
 * @section genDesc General Description
 * En este proyecto mediante la medición de la distancia con un HC-SR04, realizamos las siguientes acciones:
 * Mostrar distancia medida utilizando los leds,
 * Mostrar el valor de distancia en cm utilizando el display LCD,
 * Usar TEC1 para activar y detener la medición.
 * Usar TEC2 para mantener el resultado (“HOLD”).
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
 * | 01/05/2024 | Documentacion del programa                     |
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
/**
 * @brief Período de parpadeo para la medición en milisegundos.
 * @details Esta constante define el período de parpadeo utilizado para controlar la frecuencia de medición.
 *          Especifica el tiempo en milisegundos entre cada medición de distancia realizada por la tarea Task_measure.
 */
#define CONFIG_BLINK_PERIOD 1000
/**
 * @brief Período de parpadeo para la verificación de interruptores en milisegundos.
 * @details Esta constante define el período de parpadeo utilizado para verificar el estado de los interruptores
 *          en la tarea Task_switches. Especifica el tiempo en milisegundos entre cada lectura de los interruptores.
 */
#define CONFIG_BLINK_PERIOD_2 50

/*==================[internal data definition]===============================*/
bool start = false;
bool hold = false;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Controla los LEDs según la distancia medida.
 * @details Esta función controla los LEDs según la distancia medida, siguiendo las siguientes reglas:
 *          - Si la distancia es menor a 10 cm, apaga todos los LEDs.
 *          - Si la distancia está entre 10 y 20 cm, enciende el LED_1 y apaga el LED_2 y LED_3.
 *          - Si la distancia está entre 20 y 30 cm, enciende el LED_1 y LED_2 y apaga el LED_3.
 *          - Si la distancia es mayor a 30 cm, enciende todos los LEDs.
 *
 * @param distancia La distancia medida en centímetros.
 */

void controlLED(uint16_t distancia)
{
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
/**
 * @brief Tarea para medir la distancia y mostrarla.
 * @details Esta tarea cuando el bool "start" es verdadero se encarga de medir la distancia utilizando
 * 			un sensor ultrasónico y mostrarla en un display LCD. Además, controla el encendido y apagado de
 *          los LEDs utilizando la funcion controlLED.
 *          La tarea actualiza el display LCD con el valor de la distancia,
 *          excepto si se activa la función "hold", que mantiene el último valor mostrado.
 *
 * @param pvParameter Puntero a parámetro pasado a la tarea (no utilizado).
 */
void Task_measure(void *pvParameter)
{
	uint16_t distancia = 0;

	while (1)
	{
		if (start)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
			controlLED(distancia);
			if (!hold)
			{
				LcdItsE0803Write(distancia);
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

/**
 * @brief Tarea para leer el estado de los interruptores y controlar el inicio y la retención del display.
 * @details Esta tarea lee las teclas y al usar TEC1 activar o detenie la medición y con TEC2 mantiene el resultado (“HOLD”).
 * 
 * @param pvParameter Puntero a parámetro pasado a la tarea (no utilizado).
 */
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
	//Inicializo periféricos
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();

	xTaskCreate(&Task_measure, "Task_measure", 2048, NULL, 5, NULL);
	xTaskCreate(&Task_switches, "Task_switches", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/