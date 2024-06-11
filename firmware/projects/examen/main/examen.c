/*! @mainpage Examen:irrigación automática de plantas

 *
 * @section genDesc General Description
 *
 * Este proyecto trata de un  dispositivo basado en la ESP-EDU que permite
 * controlar el riego y el pH de una plantera.
 *
 *
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
 * | 11/06/2024 | Document creation		                         |
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
#include "switch.h"

#include "timer_mcu.h"

#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_TIMER_HUMEDAD 3000
#define CONFIG_TIMER_PH 3000
#define GPIO_BOMBA GPIO_23
#define GPIO_BOMBA_ACIDO GPIO_22
#define GPIO_BOMBA_BASICO GPIO_21

#define GPIO_SENSOR_HUMEDAD GPIO_17

/*==================[internal data definition]===============================*/
TaskHandle_t humedad_task_handle = NULL;
TaskHandle_t ph_task_handle = NULL;
bool start;

/*==================[internal functions declaration]=========================*/



/**
 * @brief Activa la bombas para agregar agua y  soluciones.
 */
void PrenderBomba(int bomba)
{switch (bomba)
{
case 1:
	GPIOOn(GPIO_BOMBA); // Prender Bombaa
	break;
case 2:
	GPIOOn(GPIO_BOMBA_ACIDO); // Prender Bombaa
	break;
case 3:
	GPIOOn(GPIO_BOMBA_BASICO); // Prender Bombaa
	break;
}
	
}
/**
 * @brief Desactiva la bomba para detener el suministro de agua y soluciones.
 */
void ApagarBomba(int bomba)
{switch (bomba)
{
case 1:
	GPIOOff(GPIO_BOMBA);
	break;
case 2:
	GPIOOff(GPIO_BOMBA_ACIDO);
	break;
case 3:
	GPIOOff(GPIO_BOMBA_BASICO); 
	break;
}
}
/**
 * @brief Interrupcion que Detiene el sistema.
 */
void FunctionStop()
{
	start = false;
}
/**
 * @brief Interrupcion que inicia.
 */
void FunctionStart()
{
	start = true;
}
/**
 * @brief Tarea para controlar la humedad de la plantera.
 * @param pvParameter Parámetro de la tarea.
 */
void Task_Control_Humedad(void *pvParameter)
{

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(start){
			if(GPIORead(GPIO_SENSOR_HUMEDAD)){
				PrenderBomba(1);
			}
			else ApagarBomba(1);

		}
	}
}

/**
 * @brief Tarea para controlar el ph de la plantera.
 * @param pvParameter Parámetro de la tarea.
 */
void Task_Control_Ph(void *pvParameter)
{

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(start){



		}
	}
}

/**
 * @brief Función de callback para el timer de control de humedad.
 * @param param Parámetro de la función.
 */
void FunctionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(humedad_task_handle, pdFALSE);
}
/**
 * @brief Función de callback para el timer de control de ph.
 * @param param Parámetro de la función.
 */
void FunctionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(ph_task_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{	
	
	// Para la bomba
	GPIOInit(GPIO_BOMBA, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA);

	GPIOInit(GPIO_BOMBA_ACIDO, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_ACIDO);

	GPIOInit(GPIO_BOMBA_BASICO, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_BASICO);
	//Para el sensor de humedad
	GPIOInit(GPIO_SENSOR_HUMEDAD, GPIO_INPUT);
	

	SwitchActivInt(SWITCH_1, &FunctionStart, NULL);
	SwitchActivInt(SWITCH_2, &FunctionStop, NULL);

	// Configuración del timer para control de humedad
	timer_config_t timer_humedad = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_HUMEDAD,
		.func_p = FunctionTimerA,
		.param_p = NULL};
	TimerInit(&timer_humedad);
	// Configuración del timer para control de ph
	timer_config_t timer_ph = {
		.timer = TIMER_B,
		.period = CONFIG_TIMER_PH,
		.func_p = FunctionTimerB,
		.param_p = NULL};
	TimerInit(&timer_ph);

	xTaskCreate(&Task_Control_Humedad, "Task_humedad", 2048, NULL, 5, &humedad_task_handle);
	xTaskCreate(&Task_Control_Ph, "Task_ph", 2048, NULL, 5, &ph_task_handle);

	TimerStart(timer_humedad.timer);
	TimerStart(timer_ph.timer);
}
/*==================[end of file]============================================*/