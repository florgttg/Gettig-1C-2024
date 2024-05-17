/*! @mainpage Template
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
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 17/05/2024 | Se pone emulador bluetooth HID para simular teclado|		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
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
//#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
#define DELAY_MEASURE 50
#define LED_BT LED_1
/*==================[internal data definition]===============================*/
bool click = false;

/*==================[internal functions declaration]=========================*/
/**
 * @brief
 *
 */
void FuncTecla1(void)
{
	keyboard_cmd_t space = HID_KEY_SPACEBAR;
	BleHidSendKeyboard(0, &space, 1);
}
/**
 * @brief
 *
 */
void FuncTecla2(void)
{
	keyboard_cmd_t down = HID_KEY_DOWN_ARROW;
	BleHidSendKeyboard(0, &down, 1);
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	SwitchesInit();
	SwitchActivInt(SWITCH_1, FuncTecla1, 0);
	SwitchActivInt(SWITCH_2, FuncTecla2, 0);
	BleHidInit("EP_HID");

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