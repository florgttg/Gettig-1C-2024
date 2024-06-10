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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
#define DELAY_MEASURE 400
#define LED_BT LED_1

/*==================[internal data definition]===============================*/
bool click = false;
TaskHandle_t acel_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función que envía un comando de teclado de la tecla espacio a través de Bluetooth.
 *
 */
void FuncTecla0(void)
{
	// keyboard_cmd_t space = HID_KEY_SPACEBAR;
	// BleHidSendKeyboard(0, &space, 1);
	ADXL335Calibration();
}
/**
 * @brief Función que envía un comando de teclado de la tecla flecha arriba a través de Bluetooth.
 *
 */
void FuncTecla1(void)
{
	keyboard_cmd_t up = HID_KEY_UP_ARROW;
	BleHidSendKeyboard(0, &up, 1);
}
/**
 * @brief Función que envía un comando de teclado de la tecla flecha abajo a través de Bluetooth.
 *
 */
void FuncTecla2(void)
{
	keyboard_cmd_t down = HID_KEY_DOWN_ARROW;
	BleHidSendKeyboard(0, &down, 1);
}
void AcelerometroTask(void *pvParameter)
{
	float acel_x0 = 0, acel_y0 = 0, acel_z0 = 0;
	float acel_x, acel_y, acel_z;
	float umbral_y = 100;
	while (1)
	{
		// if (BleHidStatus() == BLE_CONNECTED)
		{
			acel_x = ReadXValue();
			acel_y = ReadYValue();
			acel_z = ReadZValue();

			if ((acel_y - acel_y0) > umbral_y)
			{
				FuncTecla2();
				acel_y0 = acel_y;
			}
			else if ((acel_y0 - acel_y) > umbral_y)
{
				FuncTecla1();
			acel_y0 = acel_y;
		}
		else(acel_y0 = acel_y);
		printf("el valor de x %f,y %f,z %f   \n", acel_x, acel_y, acel_z);
	}
	vTaskDelay(DELAY_MEASURE / portTICK_PERIOD_MS);
}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	// Teclas
	SwitchesInit();
	SwitchActivInt(SWITCH_1, FuncTecla0, 0);
	SwitchActivInt(SWITCH_2, FuncTecla2, 0);
	// Inicializa bluetooth
	BleHidInit("EP_HID");

	ADXL335Init();
	xTaskCreate(&AcelerometroTask, "ACELEROMETRO", 4096, NULL, 5, &acel_task_handle);

	// Chequea que el bluetooth este correctamente conectado con el led1
	while (1)
	{
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
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
	}
}

/*==================[end of file]============================================*/