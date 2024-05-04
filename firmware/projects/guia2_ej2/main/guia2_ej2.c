/*! @mainpage Guia de proyecto numero 2
 *
 * @section genDesc General Description
 * Utilizamos interrupciones y timers.
 *
 * En este proyecto mediante la medición de la distancia con un HC-SR04, realizamos las siguientes acciones:
 * Mostrar distancia medida utilizando los leds,
 * Mostrar el valor de distancia en cm utilizando el display LCD,
 * Usar TEC1 para activar y detener la medición.
 * Usar TEC2 para mantener el resultado (“HOLD”).
 *
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
 * | 01/05/2024 |Documentacion                                   |
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
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/

#define CONFIG_TIMER_PERIOD 1000000
/*==================[internal data definition]===============================*/
/**
 * @brief Indicador del estado de inicio de la medición.
 * Esta variable global booleana indica si la medición está en curso (true) o no (false).
 */
bool start = false;
/**
 * @brief Indicador del estado de retención de la medición.
 * Esta variable global booleana indica si la medición está en estado de retención (true) o no (false).
 */
bool hold = false;
/**
 * @brief Identificador de la tarea de medición.
 */
TaskHandle_t measure_task_handle = NULL;

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
 * @details Esta tarea cuando recibe una notificacion y si el bool "start" es verdadero se encarga de medir la distancia utilizando
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
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (start)
		{
			distancia = HcSr04ReadDistanceInCentimeters();

			if (!hold)
			{
				LcdItsE0803Write(distancia);
			}

			controlLED(distancia);
		}
		else
		{
			LedsOffAll();
			LcdItsE0803Off();
		}
		//		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}
/**
 * @brief Función para cambiar el estado de hold.
 * Se utiliza para manejo de interrupciones
 */
void FunctionHold()
{
	hold = !hold;
}
/**
 * @brief Función para cambiar el estado de start.
 */
void FunctionStart()
{
	start = !start;
}
/**
 *  @brief Función de callback del Timer que envia la notificación
 * a la tarea Task_measure para que realice una medición de distancia.
 */
void FunctionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(measure_task_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();

	// ESTO ES CON TAREAS
	//	xTaskCreate(&Task_measure, "Task_measure", 2048, NULL, 5, NULL);
	//  xTaskCreate(&Task_switches, "Task_switches", 512, NULL, 5, NULL);

	// ESTO CON INTERRUPCIONES
	SwitchActivInt(SWITCH_1, &FunctionStart, NULL);
	SwitchActivInt(SWITCH_2, &FunctionHold, NULL);
	// ESTO ES CON TIMERS
	timer_config_t timer_start = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_PERIOD,
		.func_p = FunctionTimerB,
		.param_p = NULL};
	TimerInit(&timer_start);

	xTaskCreate(&Task_measure, "Task_measure", 2048, NULL, 5, &measure_task_handle);

	TimerStart(timer_start.timer);
}
/*==================[end of file]============================================*/
