/*! @mainpage Proyecto Integrador
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
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                        |
 * |:----------:|:---------------------------------------------------|
 * | 17/05/2024 | Se pone emulador bluetooth HID para simular teclado|
 *
 * @author Florencia Gettig (florenci.gettig@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gpio_mcu.h"
#include "led.h"
#include "ble_hid_mcu.h"
#include "switch.h"
#include "ADXL335.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_TIMER_VUELO 400
#define CONFIG_TIMER_CONTROLES 1000



#define LED_BT LED_1
#define LED_START LED_3

/*==================[internal data definition]===============================*/
bool start = false;

TaskHandle_t volar_task_handle = NULL;
TaskHandle_t controles_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función que envía un comando de teclado de la barra espaciadora a través de Bluetooth.
 *
 */
void TeclaBarra(void)
{
	keyboard_cmd_t up = HID_KEY_UP_ARROW;
	BleHidSendKeyboard(0, &up, 1);
	start = !start;
}
/**
 * @brief Función que calibra el acelerometro.
 *
 */
void Calibrar(void)
{
	ADXL335Calibration();
}
/**
 * @brief Función que envía un comando de teclado de la tecla flecha arriba a través de Bluetooth.
 *
 */
void TeclaArriba(void)
{
	keyboard_cmd_t up = HID_KEY_UP_ARROW;
	BleHidSendKeyboard(0, &up, 1);
}
/**
 * @brief Función que envía un comando de teclado de la tecla flecha abajo a través de Bluetooth.
 *
 */
void TeclaAbajo(void)
{
	keyboard_cmd_t down = HID_KEY_DOWN_ARROW;
	BleHidSendKeyboard(0, &down, 1);
}
/**
 * @brief Función que recibe la lectura del acelerometro.
 *
 */
void Task_Volar(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (start)
		{
		}
	}
}

void Task_Controles(void *pvParameter)
{
	// Chequea que el bluetooth este correctamente conectado con el led1
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		switch (BleHidStatus())
        {
        case BLE_OFF:
            LedOff(LED_BT);
            break;
        case BLE_DISCONNECTED:
            LedToggle(LED_BT);
            break;
        case BLE_CONNECTED:
            LedOn(LED_BT);
            break;
        }


        if (start)
        {
            LedOn(LED_START);
        }
        else
        {
            LedOff(LED_START);
        }


	}
}

/**
 * @brief Función de callback para el timer A.
 * @param param Parámetro de la función.
 */
void FunctionTimerA(void *param)
{
	vTaskNotifyGiveFromISR(volar_task_handle, pdFALSE);
}
/**
 * @brief Función de callback para el timer B.
 * @param param Parámetro de la función.
 */
void FunctionTimerB(void *param)
{
	vTaskNotifyGiveFromISR(controles_task_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	ADXL335Init();
	SwitchesInit();
	BleHidInit("EP_HID"); // Inicializa bluetooth

	// ESTO CON INTERRUPCIONES
	SwitchActivInt(SWITCH_1, &Calibrar, NULL);
	SwitchActivInt(SWITCH_2, &TeclaBarra, NULL);

	// Configuración del timer para control del vuelo del pajaro
	timer_config_t timer_vuelo = {
		.timer = TIMER_A,
		.period = CONFIG_TIMER_VUELO,
		.func_p = FunctionTimerA,
		.param_p = NULL};
	TimerInit(&timer_vuelo);

	// Configuración del timer para control de bluetooth
	timer_config_t timer_controles = {
		.timer = TIMER_B,
		.period = CONFIG_TIMER_CONTROLES,
		.func_p = FunctionTimerB,
		.param_p = NULL};
	TimerInit(&timer_controles);

	xTaskCreate(&Task_Volar, "Task_Volar", 4096, NULL, 5, &volar_task_handle);
	xTaskCreate(&Task_Controles, "Task_control", 2048, NULL, 4, &controles_task_handle);

	TimerStart(timer_vuelo.timer);
	TimerStart(timer_controles.timer);
}

/*==================[end of file]============================================*/