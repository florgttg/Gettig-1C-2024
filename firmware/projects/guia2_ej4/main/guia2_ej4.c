/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este es el ejercicio 4 del proyecto 2
 * Diseño e implementacion una aplicación, basada en el driver analog_io_mcu.h y el driver de
 * transmisión serie uart_mcu.h, que digitalice una señal analógica y la transmita a un graficador
 * de puerto serie de la PC. Se debe tomar la entrada CH1 del conversor AD y la transmisión se
 * debe realizar por la UART conectada al puerto serie de la PC, en un formato compatible con un
 * graficador por puerto serie.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 *
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define BUFFER_SIZE 231
#define CONFIG_TIMER_A_PERIOD 2000

TaskHandle_t analog_task_handle = NULL;
/*==================[internal data definition]===============================*/
TaskHandle_t main_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
	76,
	77,
	78,
	77,
	79,
	86,
	81,
	76,
	84,
	93,
	85,
	80,
	89,
	95,
	89,
	85,
	93,
	98,
	94,
	88,
	98,
	105,
	96,
	91,
	99,
	105,
	101,
	96,
	102,
	106,
	101,
	96,
	100,
	107,
	101,
	94,
	100,
	104,
	100,
	91,
	99,
	103,
	98,
	91,
	96,
	105,
	95,
	88,
	95,
	100,
	94,
	85,
	93,
	99,
	92,
	84,
	91,
	96,
	87,
	80,
	83,
	92,
	86,
	78,
	84,
	89,
	79,
	73,
	81,
	83,
	78,
	70,
	80,
	82,
	79,
	69,
	80,
	82,
	81,
	70,
	75,
	81,
	77,
	74,
	79,
	83,
	82,
	72,
	80,
	87,
	79,
	76,
	85,
	95,
	87,
	81,
	88,
	93,
	88,
	84,
	87,
	94,
	86,
	82,
	85,
	94,
	85,
	82,
	85,
	95,
	86,
	83,
	92,
	99,
	91,
	88,
	94,
	98,
	95,
	90,
	97,
	105,
	104,
	94,
	98,
	114,
	117,
	124,
	144,
	180,
	210,
	236,
	253,
	227,
	171,
	99,
	49,
	34,
	29,
	43,
	69,
	89,
	89,
	90,
	98,
	107,
	104,
	98,
	104,
	110,
	102,
	98,
	103,
	111,
	101,
	94,
	103,
	108,
	102,
	95,
	97,
	106,
	100,
	92,
	101,
	103,
	100,
	94,
	98,
	103,
	96,
	90,
	98,
	103,
	97,
	90,
	99,
	104,
	95,
	90,
	99,
	104,
	100,
	93,
	100,
	106,
	101,
	93,
	101,
	105,
	103,
	96,
	105,
	112,
	105,
	99,
	103,
	108,
	99,
	96,
	102,
	106,
	99,
	90,
	92,
	100,
	87,
	80,
	82,
	88,
	77,
	69,
	75,
	79,
	74,
	67,
	71,
	78,
	72,
	67,
	73,
	81,
	77,
	71,
	75,
	84,
	79,
	77,
	77,
	76,
	76,
};

/*==================[internal functions declaration]=========================*/

/**
 * @brief Toma un valor analogico y lo convierte en digital
*/
static void Task_analog_to_DC(void *pvParameter)
{
	uint16_t valorDigital;
	while (1)
	{   
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);//Recibo la notificacion del timer

		AnalogInputReadSingle(CH1,&valorDigital);//Lo leo

		UartSendString(UART_PC, (char *)UartItoa(valorDigital, 10));
		UartSendString(UART_PC, "\r\n");


	}
}

void FunctionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(analog_task_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	// Analog

	analog_input_config_t senal_canal_1 = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL};
	AnalogInputInit(&senal_canal_1);

	// Timers
	timer_config_t timer_start = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_A_PERIOD,
		.func_p = FunctionTimerA,
		.param_p = NULL};
	TimerInit(&timer_start);

	//UART 
	//Esto del uart(puerto serie)
	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200,		 /*!< baudrate (bits per second) */
		.func_p = NULL, /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL			 /*!< Pointer to callback function parameters */
	};
	UartInit(&my_uart);


	// Tarea
	xTaskCreate(&Task_analog_to_DC, "Task_analog", 2048, NULL, 5, &analog_task_handle);
	TimerStart(timer_start.timer);

}
/*==================[end of file]============================================*/