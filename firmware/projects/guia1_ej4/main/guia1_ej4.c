/*! @mainpage Guia Proyecto 1_Ejercicios 4 5 y 6
 *
 * @section genDesc General Description
 *
 *El programa permite mostrar a traves de un display 3 digitos especificados.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |   Display lcd  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/03/2024 | Document creation		                         |
 *
 * @author Florencia Gettig 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @brief Estructura que guarda el numero de pin de gpio y si es de entrada o salida.
 */
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
//4
/**
 * @brief Convierte un número entero en su representación BCD (Binary-Coded Decimal).
 *
 * Esta función toma un número entero y lo convierte en un arreglo de dígitos BCD.
 *
 * @param data El número entero a convertir.
 * @param digits La cantidad de dígitos en el número.
 * @param bcd_number El arreglo donde se almacenará la representación BCD.
 * @return 0 si la conversión fue exitosa.
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (int i = digits - 1; i >= 0; --i)
	{
		bcd_number[i] = data % 10;
		data /= 10;
	}
	return 0;
}
//5
/**
 * @brief Establece el estado de los pines GPIO según un valor en código binario decimal (BCD).
 *
 * Esta función toma un valor BCD (que representa un número binario de 4 bits ) y configura los pines GPIO
 * correspondientes en el arreglo `gpioArray` según los bits individuales del valor BCD.
 *
 * @param bcd El valor BCD a interpretar (8 bits).
 * @param gpioArray Un arreglo de estructuras `gpioConf_t` que representa los pines GPIO.
 */
void setGpioState(uint8_t bcd, gpioConf_t *gpioArray)
{
	for (int i = 0; i < 4; i++)
	{
		if ((bcd >> i) & 1)
		{
			GPIOOn(gpioArray[i].pin);
		}
		else
			GPIOOff(gpioArray[i].pin);
	}
}
//6
/**
 * @brief Muestra datos en un display de 7 segmentos.
 *
 * Esta función toma un número entero sin signo de 32 bits (`data`), la cantidad de dígitos a mostrar (`digits`),
 * un arreglo de configuración de pines GPIO (`gpioArray`), otro arreglo de configuración de pines GPIO (`gpioMap`),
 * y un puntero a un arreglo de números en formato BCD (`bcd_number`).
 *
 * @param data     El número que se mostrará en el display.
 * @param digits   La cantidad de dígitos en el display.
 * @param gpioArray Arreglo de configuración de pines GPIO para controlar los segmentos del display.
 * @param gpioMap  Arreglo de configuración de pines GPIO para controlar los dígitos del display.
 * @param bcd_number Puntero al arreglo de números en formato BCD.
 */
void mostrarDisplay ( uint32_t data, uint8_t digits, gpioConf_t *gpioArray,  gpioConf_t *gpioMap ,uint8_t *bcd_number )
{
	convertToBcdArray(data,digits,bcd_number);
	for (int i=0;i<digits;i++){
			setGpioState(bcd_number[i],gpioArray);
			GPIOOn(gpioMap[i].pin);
			GPIOOff(gpioMap[i].pin);
	}
	
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t numero = 123;
	uint8_t digits = 3;
	uint8_t bcd_number[digits];

	//convertToBcdArray(numero, digits, bcd_number);
	gpioConf_t gpioArray[4] =
		{
			{GPIO_20, GPIO_OUTPUT},
			{GPIO_21, GPIO_OUTPUT}, // sale porque va a ala pantalla
			{GPIO_22, GPIO_OUTPUT},
			{GPIO_23, GPIO_OUTPUT}}; // Declara un vector de 4 estructuras gpioConf_t

	//inicializo los gpio
	for (int i = 0; i < 4; i++)
	{
		GPIOInit(gpioArray[i].pin, gpioArray[i].dir);
	}
	//setGpioState(bcd_number[1],gpioArray);

gpioConf_t gpioMap[3] =
		{
			{GPIO_19, GPIO_OUTPUT},
			{GPIO_18, GPIO_OUTPUT}, // sale porque va a ala pantalla
			{GPIO_9, GPIO_OUTPUT},
		};

	//inicializo los gpio
	for(int i = 0; i < 3; i++)
	{
		GPIOInit(gpioMap[i].pin, gpioMap[i].dir);
	};
mostrarDisplay(numero, digits,gpioArray,gpioMap,bcd_number);


}
/*==================[end of file]============================================*/