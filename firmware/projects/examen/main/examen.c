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
 *  *
 *| Peripheral          | ESP32  |
 *|:-------------------:|:------:|
 *| Bomba de Agua       | GPIO_23|
 *| Bomba de PhA        | GPIO_22|
 *| Bomba de PhB        | GPIO_21|
 *| Sensor de Ph        | CH1    |
 *| Sensor de Humedad   | GPIO_17|
 *
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
#define CONFIG_TIMER_UART 5000
#define CONFIG_TIMER_PH 3000
#define GPIO_BOMBA GPIO_23
#define GPIO_BOMBA_ACIDO GPIO_22
#define GPIO_BOMBA_BASICO GPIO_21

#define GPIO_SENSOR_HUMEDAD GPIO_17
#define GPIO_SENSOR_PH CH1


#define VOLTAJE_MAX 3000

#define PH_MAX 14
#define PH_ACIDO 6.7
#define PH_BASICO 6
/*==================[internal data definition]===============================*/
TaskHandle_t uart_task_handle = NULL;//Controlador de tarea de mensajes
TaskHandle_t humedad_ph_task_handle = NULL;//controlador de tareas de control de humedad y ph
bool start;//bool para inicio o no del sistema

bool bomba_phA=false;
bool bomba_phB=false;
bool bomba_agua=false;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Activa la bombas para agregar agua y  soluciones.
 * @param bomba bomba que se va a activar
 */
void PrenderBomba(int bomba)
{
	switch (bomba)
	{
	case 1:
		GPIOOn(GPIO_BOMBA);
		bomba_agua = true; // Prender Bombaa
		break;
	case 2:
		GPIOOn(GPIO_BOMBA_ACIDO); // Prender Bombaa
		bomba_phA = true;
		break;
	case 3:
		GPIOOn(GPIO_BOMBA_BASICO); // Prender Bombaa
		bomba_phB = true;
		break;
	}
}
/**
 * @brief Desactiva la bomba para detener el suministro de agua y soluciones.
 * @param bomba bomba que se va a desactivar
 */
void ApagarBomba(int bomba)
{
	switch (bomba)
	{
	case 1:
		GPIOOff(GPIO_BOMBA);
		bomba_agua = false;
		break;
	case 2:
		GPIOOff(GPIO_BOMBA_ACIDO);
		bomba_phA = false;
		break;
	case 3:
		GPIOOff(GPIO_BOMBA_BASICO);
		bomba_phB = false;
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
 * @brief Convierte un valor de voltaje a ph.
 * @param voltaje Valor del voltaje en milivoltios.
 * @return Valor del ph.
 */
uint16_t ConvertirVoltajeAPh(uint16_t voltaje)
{
	return voltaje * PH_MAX / VOLTAJE_MAX;
}
/**
 * @brief Tarea para enviar mensajes por la uart.
 * @param pvParameter Parámetro de la tarea.
 */
void Task_Mensajes(void *pvParameter)
{

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (start)
		{
			if (bomba_agua)
			{
				UartSendString(UART_PC, "Bomba de agua encendida");
			}
			else
			{
				UartSendString(UART_PC, "Humedad Correcta");
			}

			if (bomba_phA)
			{
				UartSendString(UART_PC, "Bomba de solucion acida encendida");
			}
			else if (bomba_phB)
			{
				UartSendString(UART_PC, "Bomba de solucion basica encendida");
			}
			else
			{
				UartSendString(UART_PC, "ph Correcto");
			}
		}
	}
}

/**
 * @brief Tarea para controlar la humedad y el ph de la plantera.
 * @param pvParameter Parámetro de la tarea.
 */
void Task_Control_Humedad_y_Ph(void *pvParameter)
{
	uint16_t phMv;
	uint16_t ph;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (start)
		{
			if (GPIORead(GPIO_SENSOR_HUMEDAD))
			{
				PrenderBomba(1);
			}
			else
				ApagarBomba(1);
		}
		AnalogInputReadSingle(GPIO_SENSOR_PH, &phMv); // Lee el valor del ph en voltaje
		ph = ConvertirVoltajeAPh(phMv);
		if (ph < PH_BASICO)
		{
			PrenderBomba(3);
		}
		else
		{
			ApagarBomba(3);
		}
		if (ph > PH_ACIDO)
		{
			PrenderBomba(2);
		}
		else
		{
			ApagarBomba(2);
		}
	}
}


/**
 * @brief Función de callback para el timer para enviar mensajes.
 * @param param Parámetro de la función.
 */
void FunctionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(uart_task_handle, pdFALSE);
}
/**
 * @brief Función de callback para el timer de control de humedad y ph.
 * @param param Parámetro de la función.
 */
void FunctionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(humedad_ph_task_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{	SwitchesInit();

	// Para la bomba
	GPIOInit(GPIO_BOMBA, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA);

	GPIOInit(GPIO_BOMBA_ACIDO, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_ACIDO);

	GPIOInit(GPIO_BOMBA_BASICO, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_BASICO);
	// Para el sensor de humedad
	GPIOInit(GPIO_SENSOR_HUMEDAD, GPIO_INPUT);

	// Para el sensor de ph
	analog_input_config_t senal_ph = {
		.input = GPIO_SENSOR_PH,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL};

	AnalogInputInit(&senal_ph);

	// Para la uart
	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200, /*!< baudrate (bits per second) */
		.func_p = NULL,		 /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL		 /*!< Pointer to callback function parameters */
	};
	UartInit(&my_uart);

	// Utilizo teclas para comenzar y parar sistema
	SwitchActivInt(SWITCH_1, &FunctionStart, NULL);
	SwitchActivInt(SWITCH_2, &FunctionStop, NULL);

	// Configuración del timer para control de humedad
	timer_config_t timer_uart = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_UART,
		.func_p = FunctionTimerA,
		.param_p = NULL};
	TimerInit(&timer_uart);
	
	// Configuración del timer para control de ph
	timer_config_t timer_humedad_y_ph = {
		.timer = TIMER_B,
		.period = CONFIG_TIMER_PH,
		.func_p = FunctionTimerB,
		.param_p = NULL};
	TimerInit(&timer_humedad_y_ph);

	xTaskCreate(&Task_Mensajes, "Task_UART", 2048, NULL, 5, &uart_task_handle);
	xTaskCreate(&Task_Control_Humedad_y_Ph, "Task_ph", 2048, NULL, 5, &humedad_ph_task_handle);

	TimerStart(timer_uart.timer);
	TimerStart(timer_humedad_y_ph.timer);
}
/*==================[end of file]============================================*/