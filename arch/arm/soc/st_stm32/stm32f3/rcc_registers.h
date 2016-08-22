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

#ifndef _STM32F3X_CLOCK_H_
#define _STM32F3X_CLOCK_H_

/**
 * @brief Driver for Reset & Clock Control of STM32F3X family processor.
 *
 * Based on reference manual:
 *  RM0316 STM32F303xB/C/D/E, STM32F303x6/8, STM32F328x8, STM32F358xC,
 *  STM32F398xE advanced ARM ® -based MCUs
 *
 * Chapter 9: Reset and clock control (RCC)
 */

enum {
	STM32F3X_RCC_CFG_PLL_SRC_HSI	  = 0x0,
	STM32F3X_RCC_CFG_PLL_SRC_HSE     = 0x1,
};

enum {
	STM32F3X_RCC_CFG_PLL_XTPRE_DIV_0  = 0x0,
	STM32F3X_RCC_CFG_PLL_XTPRE_DIV_2  = 0x1,
};

enum {
	STM32F3X_RCC_CFG_SYSCLK_SRC_HSI = 0x0,
	STM32F3X_RCC_CFG_SYSCLK_SRC_HSE = 0x1,
	STM32F3X_RCC_CFG_SYSCLK_SRC_PLL = 0x2,
};

enum {
	STM32F3X_RCC_CFG_HCLK_DIV_0  = 0x0,
	STM32F3X_RCC_CFG_HCLK_DIV_2  = 0x4,
	STM32F3X_RCC_CFG_HCLK_DIV_4  = 0x5,
	STM32F3X_RCC_CFG_HCLK_DIV_8  = 0x6,
	STM32F3X_RCC_CFG_HCLK_DIV_16 = 0x7,
};

enum {
	STM32F3X_RCC_CFG_SYSCLK_DIV_0	 = 0x0,
	STM32F3X_RCC_CFG_SYSCLK_DIV_2	 = 0x8,
	STM32F3X_RCC_CFG_SYSCLK_DIV_4	 = 0x9,
	STM32F3X_RCC_CFG_SYSCLK_DIV_8	 = 0xa,
	STM32F3X_RCC_CFG_SYSCLK_DIV_16	 = 0xb,
	STM32F3X_RCC_CFG_SYSCLK_DIV_64	 = 0xc,
	STM32F3X_RCC_CFG_SYSCLK_DIV_128 = 0xd,
	STM32F3X_RCC_CFG_SYSCLK_DIV_256 = 0xe,
	STM32F3X_RCC_CFG_SYSCLK_DIV_512 = 0xf,
};

/* 9.4.1 Clock control register (RCC_CR) */
union __rcc_cr {
	uint32_t val;
	struct {
		uint32_t hsion :1 __packed;
		uint32_t hsirdy :1 __packed;
		uint32_t rsvd__2 :1 __packed;
		uint32_t hsitrim :5 __packed;
		uint32_t hsical :8 __packed;
		uint32_t hseon :1 __packed;
		uint32_t hserdy :1 __packed;
		uint32_t hsebyp :1 __packed;
		uint32_t csson :1 __packed;
		uint32_t rsvd__20_23 :4 __packed;
		uint32_t pllon :1 __packed;
		uint32_t pllrdy :1 __packed;
		uint32_t rsvd__26_31 :6 __packed;
	} bit;
};

/* 9.4.2 Clock configuration register (RCC_CFGR)*/
union __rcc_cfgr {
	uint32_t val;
	struct {
		uint32_t sw :2 __packed;
		uint32_t sws :2 __packed;
		uint32_t hpre :4 __packed;
		uint32_t ppre1 :3 __packed;
		uint32_t ppre2 :3 __packed;
		uint32_t rsvd__14_15: 2 __packed;
		uint32_t pllsrc :1 __packed;
		uint32_t pllxtpre :1 __packed;
		uint32_t pllmul :4 __packed;
		uint32_t usbpre :1 __packed;
		uint32_t i2ssrc :1 __packed;
		uint32_t mco :3 __packed;
		uint32_t rsvd__27 :1 __packed;
		uint32_t mcof :1 __packed;
		uint32_t mcopre :2 __packed;
		uint32_t pplnodiv :1 __packed;
	} bit;
};

/* 9.4 RCC registers */
struct stm32f3x_rcc {
	union __rcc_cr cr;
	union __rcc_cfgr cfgr;
	uint32_t cir;
	uint32_t apb2rstr;
	uint32_t apb1rstr;
	uint32_t ahbenr;
	uint32_t apb2enr;
	uint32_t apb1enr;
	uint32_t bdcr;
	uint32_t csr;
	uint32_t ahbrstr;
	uint32_t cfgr2;
	uint32_t cfgr3;
};

#endif /* _STM32F3X_CLOCK_H_ */
