/*
 * gpio.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef GPIO_H_
#define GPIO_H_

#define GPIO_SWPORTA_DR         0x00
#define GPIO_SWPORTA_DDR        0x04
#define GPIO_SWPORTB_DR         0x0c
#define GPIO_SWPORTB_DDR        0x10
#define GPIO_SWPORTC_DR         0x18
#define GPIO_SWPORTC_DDR        0x1c
#define GPIO_SWPORTD_DR         0x24
#define GPIO_SWPORTD_DDR        0x28
#define GPIO_INTEN              0x30
#define GPIO_INTMASK            0x34
#define GPIO_INTTYPE_LEVEL      0x38
#define GPIO_INT_POLARITY       0x3c
#define GPIO_INTSTATUS          0x40
#define GPIO_PORTA_EOI          0x4c
#define GPIO_EXT_PORTA          0x50
#define GPIO_EXT_PORTB          0x54
#define GPIO_EXT_PORTC          0x58
#define GPIO_EXT_PORTD          0x5c

#define GPIO0_BASE_ADDR         0xf0300000
#define GPIO1_BASE_ADDR         0xf4000000

typedef enum {
	GPIO_NOR_MODE,
	GPIO_INT_MODE
} Gpio_Mode;

typedef enum {
	GPIO_LEVEL_LOW,
	GPIO_LEVEL_HIGH,
} Gpio_Level;

typedef enum {
	GPIO_DIR_IN,
	GPIO_DIR_OUT
} Gpio_Direction;

typedef enum {
	GPIO_INT_YYPE_LEVEL,
	GPIO_INT_YYPE_EDGE
} Gpio_Int_Pol;

typedef enum {
	GPIO_INT_POL_FALL,
	GPIO_INT_POL_RISK
} Gpio_Int_Sens;

typedef enum {
	GPIO_INT_UNTRIGGERED,
	GPIO_INT_TRIGGERED
} Gpio_Int_Status;

enum {
	GPIO_EVENT_DOWN,
	GPIO_EVENT_UP,
	GPIO_EVENT_NONE = 9999
};

enum {
	GPIO_BUTTON_UP = 15,
	GPIO_BUTTON_LEFT = 16,
	GPIO_BUTTON_DOWN = 17,
	GPIO_BUTTON_RIGHT = 18,
	GPIO_BUTTON_MENU = 19,
	GPIO_BUTTON_NONE = 999
};

void gpio_dir_set(u8 bank, int gpiono, int dir);
void gpio_data_set(u8 bank, int gpiono, int val);
unsigned int gpio_data_get(u8 bank, int gpiono);
void gpio_direction_output(int gpio, int value);

#endif /* GPIO_H_ */
