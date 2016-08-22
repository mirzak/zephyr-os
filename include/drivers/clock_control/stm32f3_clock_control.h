/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 *           (c) 2016 Mirza Krak
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _STM32F3_CLOCK_CONTROL_H_
#define _STM32F3_CLOCK_CONTROL_H_

/**
 * @file
 *
 * @brief Clock subsystem IDs for STM32F3 family
 */

/* Bus */
enum {
	STM32F3X_CLOCK_BUS_AHB1,
	STM32F3X_CLOCK_BUS_AHB2,
	STM32F3X_CLOCK_BUS_AHB3,
	STM32F3X_CLOCK_BUS_APB1,
	STM32F3X_CLOCK_BUS_APB2,
};

/* AHB1/2/3 pins */
enum {
	STM32F3X_CLOCK_ENABLE_DMA1	= 1 << 0,
	STM32F3X_CLOCK_ENABLE_DMA2	= 1 << 1,
	STM32F3X_CLOCK_ENABLE_SRAM      = 1 << 2,
	/* Bit 3 Reserved */
	STM32F3X_CLOCK_ENABLE_FLITF	= 1 << 4,
	STM32F3X_CLOCK_ENABLE_FMC	= 1 << 5,
	STM32F3X_CLOCK_ENABLE_CRC	= 1 << 6,
	/* Bit 7-15 Reserved */
	STM32F3X_CLOCK_ENABLE_GPIOH	= 1 << 16,
	STM32F3X_CLOCK_ENABLE_GPIOA	= 1 << 17,
	STM32F3X_CLOCK_ENABLE_GPIOB	= 1 << 18,
	STM32F3X_CLOCK_ENABLE_GPIOC	= 1 << 19,
	STM32F3X_CLOCK_ENABLE_GPIOD	= 1 << 20,
	STM32F3X_CLOCK_ENABLE_GPIOE	= 1 << 21,
	STM32F3X_CLOCK_ENABLE_GPIOF	= 1 << 22,
	STM32F3X_CLOCK_ENABLE_GPIOG	= 1 << 23,
	STM32F3X_CLOCK_ENABLE_TSC	= 1 << 24,
	/* Bit 25-27 Reserved */
	STM32F3X_CLOCK_ENABLE_ADC12	= 1 << 28,
	STM32F3X_CLOCK_ENABLE_ADC34	= 1 << 29,
	/* Bit 30-31 Reserved */
};

/* APB1 pins */
enum {
	STM32F3X_CLOCK_ENABLE_TIM2	= 1 << 0,
	STM32F3X_CLOCK_ENABLE_TIM3	= 1 << 1,
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_TIM4	= 1 << 2,
	/* Bit 3 Reserved */
	STM32F3X_CLOCK_ENABLE_TIM6	= 1 << 4,
	STM32F3X_CLOCK_ENABLE_TIM7	= 1 << 5,
	/* Bit 6-10 Reserved */
	STM32F3X_CLOCK_ENABLE_WWDG	= 1 << 11,
	/* Bit 12-13 Reserved */
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_SPI2	= 1 << 14,
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_SPI3	= 1 << 15,
	STM32F3X_CLOCK_ENABLE_USART2	= 1 << 17,
	STM32F3X_CLOCK_ENABLE_USART3	= 1 << 18,
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_UART4	= 1 << 19,
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_UART5	= 1 << 20,
	STM32F3X_CLOCK_ENABLE_I2C1	= 1 << 21,
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_I2C2	= 1 << 22,
	/* STM32F303xB/C/D/E devices only */
	STM32F3X_CLOCK_ENABLE_USB	= 1 << 23,
	/* Bit 24 Reserved */
	STM32F3X_CLOCK_ENABLE_CAN1	= 1 << 25,
	/* STM32F303x6/8 and STM32F328x8 devices only */
	STM32F3X_CLOCK_ENABLE_DAC2	= 1 << 26,
	/* Bit 27 Reserved */
	STM32F3X_CLOCK_ENABLE_PWR	= 1 << 28,
	STM32F3X_CLOCK_ENABLE_DAC1	= 1 << 29,
	/* only in STM32F303xD/E devices */
	STM32F3X_CLOCK_ENABLE_I2C3	= 1 << 30,
	/* Bit 31 Reserved */
};

/* APB2 pins */
enum {
	STM32F3X_CLOCK_ENABLE_SYSCFG	= 1 << 0,
	/* Bit 1-10 Reserved */
	STM32F3X_CLOCK_ENABLE_TIM1	= 1 << 11,
	STM32F3X_CLOCK_ENABLE_SPI1	= 1 << 12,
	/* STM32F303xB/C/D/E, STM32F358xC and STM32F398xE devices only */
	STM32F3X_CLOCK_ENABLE_TIM8	= 1 << 13,
	STM32F3X_CLOCK_ENABLE_USART1	= 1 << 14,
	/* STM32F303xD/E and STM32F398xE only */
	STM32F3X_CLOCK_ENABLE_SPI4	= 1 << 15,
	STM32F3X_CLOCK_ENABLE_TIM15	= 1 << 16,
	STM32F3X_CLOCK_ENABLE_TIM16	= 1 << 17,
	STM32F3X_CLOCK_ENABLE_TIM17	= 1 << 18,
	/* Bit 19 Reserved */
	STM32F3X_CLOCK_ENABLE_TIM20	= 1 << 20,
	/* Bit 21-31 Reserved */
};

struct stm32f3x_pclken {
	uint32_t bus;
	uint32_t enr;
};

#endif /* _STM32F3_CLOCK_CONTROL_H_ */
