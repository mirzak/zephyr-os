/*
 * Copyright (c) 2016 Mirza Krak
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

/**
 * @file Interrupt numbers for STM32F3 family processors.
 *
 * Based on reference manual:
 *  RM0316 STM32F303xB/C/D/E, STM32F303x6/8, STM32F328x8, STM32F358xC,
 *  STM32F398xE advanced ARM ® -based MCUs
 *
 * Chapter 14: Interrupts and events
 */


#ifndef _STM32F3_SOC_IRQ_H_
#define _STM32F3_SOC_IRQ_H_

#define STM32F3_IRQ_WWDG		0
#define STM32F3_IRQ_PVD			1
#define STM32F3_IRQ_EXTI16		STM32F3_IRQ_PVD
#define STM32F3_IRQ_TAMPER		2
#define STM32F3_IRQ_EXTI19		STM32F3_IRQ_TAMPER
#define STM32F3_IRQ_RTC			3
#define STM32F3_IRQ_EXTI20		STM32F3_IRQ_RTC
#define STM32F3_IRQ_FLASH		4
#define STM32F3_IRQ_RCC			5

#define STM32F3_IRQ_EXTI0		6
#define STM32F3_IRQ_EXTI1		7
#define STM32F3_IRQ_EXTI2_TS		8
#define STM32F3_IRQ_EXTI3		9
#define STM32F3_IRQ_EXTI4		10

#define STM32F3_IRQ_DMA1_CH1		11
#define STM32F3_IRQ_DMA1_CH2		12
#define STM32F3_IRQ_DMA1_CH3		13
#define STM32F3_IRQ_DMA1_CH4		14
#define STM32F3_IRQ_DMA1_CH5		15
#define STM32F3_IRQ_DMA1_CH6		16
#define STM32F3_IRQ_DMA1_CH7		17

#define STM32F3_IRQ_ADC1_2		18
#define STM32F3_IRQ_USB_HP_CAN_TX	19
#define STM32F3_IRQ_USB_LP_CAN_RX0	20
#define STM32F3_IRQ_CAN_RX1		21
#define STM32F3_IRQ_CAN_SCE		22
#define STM32F3_IRQ_EXTI9_5		23

#define STM32F3_IRQ_TIM1_BRK_TIM15	24
#define STM32F3_IRQ_TIM1_UP_TIM16	25
#define STM32F3_IRQ_TIM1_TRG_COM_TIM17	26
#define STM32F3_IRQ_TIM1_CC		27
#define STM32F3_IRQ_TIM2		28
#define STM32F3_IRQ_TIM3		29
/* Available only in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */
#define STM32F3_IRQ_TIM4		30

#define STM32F3_IRQ_I2C1_EV		31
#define STM32F3_IRQ_EXTI23		STM32F3_IRQ_I2C1_EV
#define STM32F3_IRQ_I2C1_ER		32

/* I2C2/EXTI24 available only in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */
#define STM32F3_IRQ_I2C2_EV		33
#define STM32F3_IRQ_EXTI24		STM32F3_IRQ_I2C2_EV
#define STM32F3_IRQ_I2C2_ER		34

#define STM32F3_IRQ_SPI1		35
/* SPI2 available only in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */
#define STM32F3_IRQ_SPI2		36

#define STM32F3_IRQ_USART1		37
#define STM32F3_IRQ_EXTI25		STM32F3_IRQ_USART1
#define STM32F3_IRQ_USART2		38
#define STM32F3_IRQ_EXTI26		STM32F3_IRQ_USART2
#define STM32F3_IRQ_USART3		39
#define STM32F3_IRQ_EXTI28		STM32F3_IRQ_USART3

#define STM32F3_IRQ_EXTI15_10		40

#define STM32F3_IRQ_RTC_ALARM		41

/* IRQ 42-53 reserved in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */

#define STM32F3_IRQ_USB_WAKEUP		42
#define STM32F3_IRQ_EXTI18		STM32F3_IRQ_USB_WAKEUP

#define STM32F3_IRQ_TIM8_BRK		43
#define STM32F3_IRQ_TIM8_UP		44
#define STM32F3_IRQ_TIM8_TRG_COM	45
#define STM32F3_IRQ_TIM8_CC		46

#define STM32F3_IRQ_ADC3		47

/* Available only in STM32F303xD/E only */
#define STM32F3_IRQ_FSMC		48

/* 49 reserved */
/* 50 reserved */

#define STM32F3_IRQ_SPI3		51

#define STM32F3_IRQ_UART4		52
#define STM32F3_IRQ_EXTI34		STM32F3_IRQ_UART4
#define STM32F3_IRQ_UART5		53
#define STM32F3_IRQ_EXTI35		STM32F3_IRQ_UART5

#define STM32F3_IRQ_TIM6_DAC1		54
#define STM32F3_IRQ_TIM7		55
 /* DAC2 available only in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */
#define STM32F3_IRQ_TIM7_DAC2		STM32F3_IRQ_TIM7

/* IRQ 56-63 reserved in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */

#define STM32F3_IRQ_DMA2_CH1		56
#define STM32F3_IRQ_DMA2_CH2		57
#define STM32F3_IRQ_DMA2_CH3		58
#define STM32F3_IRQ_DMA2_CH4		59
#define STM32F3_IRQ_DMA2_CH5		60

#define STM32F3_IRQ_ADC4		61

/* 62 reserved */
/* 63 reserved */

/* Available only in STM32F303x6/8 and STM32F328x8 */
#define STM32F3_IRQ_COMP1_2_3		64
#define STM32F3_IRQ_EXTI_21_22_29	STM32F3_IRQ_COMP1_2_3
#define STM32F3_IRQ_COMP4_5_6		65
#define STM32F3_IRQ_EXTI_30_31_32	STM32F3_IRQ_COMP4_5_6
#define STM32F3_IRQ_COM7		66
#define STM32F3_IRQ_EXTI33		STM32F3_IRQ_COM7

/* Available only in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */
#define STM32F3_IRQ_COMP2		64
#define STM32F3_IRQ_EXTI_22		STM32F3_IRQ_COMP2
#define STM32F3_IRQ_COMP4_6		65
#define STM32F3_IRQ_EXTI_30_32		STM32F3_IRQ_COMP4_6

/* IRQ 66-80 reserved in STM32F303xB/C/D/E, STM32F358xC and STM32F398xE */

/* 67-71 reserved */

/* I2C3 Available only in STM32F303xD/E only */
#define STM32F3_IRQ_I2C3_EV		72
#define STM32F3_IRQ_I2C3_ER		73

#define STM32F3_IRQ_USB_HP		74
#define STM32F3_IRQ_USB_LP		75

/* TIM20 Available only in STM32F303xD/E only */
#define STM32F3_IRQ_TIM20_BRK		77
#define STM32F3_IRQ_TIM20_UP		78
#define STM32F3_IRQ_TIM20_TRG_COM	79
#define STM32F3_IRQ_TIM20_CC		80

#define STM32F3_IRQ_FPU			81

/* IRQ 82-84 available only in STM32F303x6/8 and STM32F328x8 */

/* 82 reserved */
/* 83 reserved */

/* SPI4 Available only in STM32F303xD/E only */
#define STM32F3_IRQ_SPI4		84

#endif	/* _STM32F3_SOC_IRQ_H_ */
