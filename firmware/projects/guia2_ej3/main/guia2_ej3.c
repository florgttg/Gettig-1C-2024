/*! @mainpage Guia de proyecto numero 2
 *
 * @section genDesc General Description
 *
 * Actividad 3 - Proyecto: Medidor de distancia por ultrasonido c/interrupciones y puerto serie
 *
 *	Modifique la actividad del punto 2 agregando ahora el puerto serie.
 *	-Envíe los datos de las mediciones para poder observarlos en un terminal en la PC,
 *	siguiendo el siguiente formato:
 *  3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
 *  -Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 *  Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
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
 * | 03/05/2024 | Creation Documentation                         |
 *
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
#include "timer_mcu.h"

#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_TIMER_PERIOD 1000000
#define UART_NO_INT 0 /*!< Flag used when no reading interruption is required */

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
 * 
 * Esta variable almacena el identificador de la tarea encargada de realizar la medición.
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
 *
 * Esta tarea se encarga de medir la distancia utilizando un sensor ultrasónico y
 * mostrarla en un display LCD. Además, envía la medición de la distancia al monitor serie a través de UART.
 *
 * @param pvParameter Puntero al parámetro de la tarea (no utilizado).
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
			//	printf("Midiendo Distancia: %d", distancia);

			if (!hold)
			{
				LcdItsE0803Write(distancia);
			}
			// LE MANDO LOS DATOS AL SERIAL MONITOR
			UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
			UartSendString(UART_PC, " cm\n\r");
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

/**
 * @brief Revisa el teclado de la PC y controla la medición.
 *
 * Esta función lee el teclado de la PC a través de UART y controla el inicio o la pausa de la medición en función de la tecla presionada. Si se presiona la tecla 'O', se inicia la medición. Si se presiona la tecla 'H', se mantiene la medición pausada.
 */
void switchFromPc()
{
	uint8_t letra;
	UartReadByte(UART_PC, &letra);
	switch (letra)
	{
	case 'O':
		FunctionStart();
		break;
	case 'H':
		FunctionHold();
		break;
	}
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
	// xTaskCreate(&Task_switches, "Task_switches", 512, NULL, 5, NULL);

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
	TimerStart(timer_start.timer);

	// Esto del uart(puerto serie)
	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 9600,		 /*!< baudrate (bits per second) */
		.func_p = &switchFromPc, /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL			 /*!< Pointer to callback function parameters */
	};
	UartInit(&my_uart);

	xTaskCreate(&Task_measure, "Task_measure", 2048, NULL, 5, &measure_task_handle);
}
/*==================[end of file]============================================*/
