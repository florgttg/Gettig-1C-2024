/*! @mainpage Practica de Examen
 *
 * @section genDesc General Description
 *
 * Este proyecto es un cebador de mate automatico
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
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "switch.h"
#include "hc_sr04.h"
#include "timer_mcu.h"

#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_TIMER_SUMINISTRO 1000000 //1 segundo
#define CONFIG_TIMER_TEMPERATURA 1000000// 1 segundo
/*==================[internal data definition]===============================*/
TaskHandle_t sumnistro_task_handle = NULL;
TaskHandle_t temperatura_task_handle = NULL;



/*==================[internal functions declaration]=========================*/
void Task_Suministrar_Agua(void *pvParameter){
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

}}
void Task_Controlar_Temperatura(void *pvParameter){
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	
}}

void FunctionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(sumnistro_task_handle, pdFALSE);
}

void FunctionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(temperatura_task_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void){


	timer_config_t timer_suministro = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_SUMINISTRO,
		.func_p = FunctionTimerA,
		.param_p = NULL};
	TimerInit(&timer_suministro);

	timer_config_t timer_temperatura = {
		.timer = TIMER_B,
		.period = CONFIG_TIMER_TEMPERATURA,
		.func_p = FunctionTimerB,
		.param_p = NULL};
	TimerInit(&timer_temperatura);

	xTaskCreate(&Task_Suministrar_Agua, "Task_suministro", 2048, NULL, 5, &sumnistro_task_handle);
	xTaskCreate(&Task_Controlar_Temperatura, "Task_temperatura", 2048, NULL, 5, &temperatura_task_handle);

	TimerStart(timer_suministro.timer);
	TimerStart(timer_temperatura.timer);
}
/*==================[end of file]============================================*/