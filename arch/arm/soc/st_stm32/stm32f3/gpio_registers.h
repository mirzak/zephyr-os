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

#ifndef _STM32F3X_GPIO_REGISTERS_H_
#define _STM32F3X_GPIO_REGISTERS_H_

/**
 * @file SoC configuration macros for the STM32F3 family processors.
 *
 * Based on reference manual:
 *  RM0316 STM32F303xB/C/D/E, STM32F303x6/8, STM32F328x8, STM32F358xC,
 *  STM32F398xE advanced ARM ® -based MCUs
 *
 * Chapter 11: General-purpose I/Os (GPIO)
 */

/* 11.4 GPIO registers - each GPIO port controls 16 pins */
struct stm32f3x_gpio {
	uint32_t mode;
	uint32_t otype;
	uint32_t ospeed;
	uint32_t pupdr;
	uint32_t idr;
	uint32_t odr;
	uint32_t bsrr;
	uint32_t lckr;
	uint32_t afr[2];
	uint32_t brr;
};

union __syscfg_exticr {
	uint32_t val;
	struct {
		uint16_t rsvd__16_31;
		uint16_t exti;
	} bit;
};

/* 12.1 SYSCFG registers */
struct stm32f3x_syscfg {
	uint32_t cfgr1;
	uint32_t rcr;
	union __syscfg_exticr exticr1;
	union __syscfg_exticr exticr2;
	union __syscfg_exticr exticr3;
	union __syscfg_exticr exticr4;
	uint32_t cfgr2;
	uint32_t cfgr3;
	uint32_t rsvd[14];
	uint32_t cfgr4;
};

#endif /* _STM32F3X_GPIO_REGISTERS_H_ */
