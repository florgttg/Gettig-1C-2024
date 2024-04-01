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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h> 
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 2
#define TOGGLE 3
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/
struct leds
{
	uint8_t n_led;      //  indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de encendido/apagado
	uint8_t periodo;    //indica el tiempo de cada ciclo
	uint8_t mode;       //ON, OFF, TOGGLE
} my_leds;
/*==================[internal functions declaration]=========================*/
void controlarLeds(struct leds *led) {
	switch (led->mode)
	{
	case ON:
		switch (led->n_led)
		{
		case 1:
	    	LedOn(LED_1);
	    break;

		case 2:
	    	 LedOn(LED_2);
	 	break;

	    case 3:
	    	 LedOn(LED_3);
	    break;
	    }
	break;
	case OFF:
		switch (led->n_led)
		{
		case 1:
	    	LedOff(LED_1);
	    break;

		case 2:
	    	 LedOff(LED_2);
	 	break;

	    case 3:
	    	 LedOff(LED_3);
	    break;
	    }
	break;
	case TOGGLE:
	    	for ( int i = 0; i< led->n_ciclos; i++)
	    	{
	    		switch (led->n_led)
	    			{
	    			 case 1:
	    				 LedToggle(LED_1);
	    			 break;
	    			 case 2:
	    				 LedToggle(LED_2);
	    			 break;
	    			 case 3:
	    			     LedToggle(LED_3);
	    			 break;
	    			 }
				 for( int j=0; j<led->periodo;j++){
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		}}
	break;
	}
}
	
/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas; //entero sin signo de 8 bits
	LedsInit();
	struct leds my_led;
	my_led.mode = TOGGLE;
	my_led.n_led = 2;
	my_led.n_ciclos = 10;
	my_led.periodo = 5;
	
	controlarLeds(&my_led);
	

}
/*==================[end of file]============================================*/