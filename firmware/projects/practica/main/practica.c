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
 *| Peripheral          | ESP32  |
 *|:-------------------:|:------:|
 *| Bomba               | GPIO_23|
 *| Resistencia         | GPIO_5 |
 *| Sensor de Temperatura | CH1   |
 *| Sensor Ultrasonico Trigger | GPIO_3 |
 *| Sensor Ultrasonico Echo    | GPIO_2 |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
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
#include "hc_sr04.h"
#include "timer_mcu.h"

#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_TIMER_SUMINISTRO 1000000	 // 1 segundo
#define CONFIG_TIMER_TEMPERATURA 1000000 // 1 segundo

#define GPIO_BOMBA GPIO_23
#define GPIO_RESISTENCIA GPIO_5
#define GPIO_SENSOR_TEMPERATURA CH1

#define VOLTAJE_MIN 0.0
#define VOLTAJE_MAX 3300
#define TEMPERATURA_MIN -10.0
#define TEMPERATURA_MAX 100.0
#define TEMPERATURA_OBJETIVO 75.0

/*==================[internal data definition]===============================*/
TaskHandle_t sumnistro_task_handle = NULL;
TaskHandle_t temperatura_task_handle = NULL;
bool start;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Activa la bomba para dispensar agua.
 */
void DispensarAgua()
{
	GPIOOn(GPIO_BOMBA); // Prender Bombaa
}
/**
 * @brief Desactiva la bomba para detener el suministro de agua.
 */
void NoDispensarAgua()
{
	GPIOOff(GPIO_BOMBA); // Apagar Bombaa
}
/**
 * @brief Envia mensajes a través de UART.
 * @param mensaje Código del mensaje a enviar.
 */
void MostrarMensaje(uint8_t mensaje)
{ // Aca lo de la uart
	switch (mensaje)
	{
	case 1:
		UartSendString(UART_PC, " Temperatura correcta…acerque el mate\r\n");
		break;
	case 2:
		UartSendString(UART_PC, " “Agua fría…espere”\r\n");
		break;
	case 3:
		UartSendString(UART_PC, " “Mate en rango…comienza a cebar”\r\n");
		break;
	case 4:
		UartSendString(UART_PC, " “Mate cebado….retirelo”\r\n");
		break;
	default:
		break;
	}
}
/**
 * @brief Interrupcion que Detiene las operaciones del cebador.
 */
void FunctionStop()
{
	start = false;
}
/**
 * @brief Inicia las operaciones del cebador.
 */
void FunctionStart()
{
	start = true;
}
/**
 * @brief Activa la resistencia para calentar el agua.
 */
void CalentarAgua()
{
	GPIOOn(GPIO_RESISTENCIA);
}
/**
 * @brief Desactiva la resistencia para calentar el agua.
 */
void NoCalentarAgua()
{
	GPIOOff(GPIO_RESISTENCIA);
}
/**
 * @brief Convierte un valor de voltaje a temperatura.
 * @param voltaje Valor del voltaje en milivoltios.
 * @return Valor de la temperatura en grados Celsius.
 */
uint16_t ConvertirVoltajeATemperatura(uint16_t voltaje)
{
	return TEMPERATURA_MIN + (voltaje - VOLTAJE_MIN) * (TEMPERATURA_MAX - VOLTAJE_MIN) / (VOLTAJE_MAX - VOLTAJE_MIN);
}
/**
 * @brief Tarea para suministrar agua basado en la distancia medida por el sensor ultrasonico.
 * @param pvParameter Parámetro de la tarea.
 */
void Task_Suministrar_Agua(void *pvParameter)
{
	uint16_t distancia = 0;
	uint8_t tiempo_entrega = 0;
	uint8_t tiempo_espera = 0;

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (start)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
			if (distancia > 5 && distancia < 10)
			{
				if (tiempo_entrega < 5)
				{
					DispensarAgua();
					if (tiempo_entrega == 0)
					{
						MostrarMensaje(1);
					}
					tiempo_entrega++;
				}
				else if (tiempo_espera < 5)
				{
					MostrarMensaje(2);
					NoDispensarAgua();
					tiempo_espera++;
				}
				else
				{
					tiempo_entrega = 0;
					tiempo_espera = 0;
				}
			}
			else
			{
				NoDispensarAgua();
			}
		}
	}
}

/**
 * @brief Tarea para controlar la temperatura del agua.
 * @param pvParameter Parámetro de la tarea.
 */
void Task_Controlar_Temperatura(void *pvParameter)
{
	uint16_t temperaturamV;
	uint16_t temperatura;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (start)
		{
			AnalogInputReadSingle(GPIO_SENSOR_TEMPERATURA, &temperaturamV); // Lee el valor de la temepratura en voltaje
			temperatura = ConvertirVoltajeATemperatura(temperaturamV);
			if (temperatura < TEMPERATURA_OBJETIVO)
			{
				CalentarAgua();
				MostrarMensaje(2);
			}
			else
			{
				NoCalentarAgua();
				MostrarMensaje(1);
			}
		}
	}
}
/**
 * @brief Función de callback para el timer de suministro de agua.
 * @param param Parámetro de la función.
 */
void FunctionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(sumnistro_task_handle, pdFALSE);
}
/**
 * @brief Función de callback para el timer de control de temperatura.
 * @param param Parámetro de la función.
 */
void FunctionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(temperatura_task_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	HcSr04Init(GPIO_3, GPIO_2);
	// Para la bomba
	GPIOInit(GPIO_BOMBA, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA);
	// Para el sensor de temperatura
	analog_input_config_t senal_temperatura = {
		.input = GPIO_SENSOR_TEMPERATURA,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL};

	AnalogInputInit(&senal_temperatura);
	// Para la resitencia
	GPIOInit(GPIO_RESISTENCIA, GPIO_OUTPUT);
	GPIOOff(GPIO_RESISTENCIA);

	// Para la uart
	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200, /*!< baudrate (bits per second) */
		.func_p = NULL,		 /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL		 /*!< Pointer to callback function parameters */
	};
	UartInit(&my_uart);

	// ESTO CON INTERRUPCIONES
	SwitchActivInt(SWITCH_1, &FunctionStart, NULL);
	SwitchActivInt(SWITCH_2, &FunctionStop, NULL);

	// Configuración del timer para control de suministro de agua
	timer_config_t timer_suministro = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_SUMINISTRO,
		.func_p = FunctionTimerA,
		.param_p = NULL};
	TimerInit(&timer_suministro);
	// Configuración del timer para control de temperatura
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