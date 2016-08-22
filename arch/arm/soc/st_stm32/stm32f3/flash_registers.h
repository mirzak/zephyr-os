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

#ifndef _STM32F3X_FLASH_REGISTERS_H_
#define _STM32F3X_FLASH_REGISTERS_H_

/**
 * @brief
 *
 * Based on reference manual:
 *  RM0316 STM32F303xB/C/D/E, STM32F303x6/8, STM32F328x8, STM32F358xC,
 *  STM32F398xE advanced ARM ® -based MCUs
 *
 * Chapter 4: Embedded Flash memory
 */

enum {
	STM32F3X_FLASH_LATENCY_0 = 0x0,
	STM32F3X_FLASH_LATENCY_1 = 0x1,
	STM32F3X_FLASH_LATENCY_2 = 0x2,
};

/* 4.5.1 FLASH_ACR */
union __ef_acr {
	uint32_t val;
	struct {
		uint32_t latency :3 __packed;
		uint32_t hlfcya :1 __packed;
		uint32_t prftbe :1 __packed;
		uint32_t prftbs :1 __packed;
		uint32_t rsvd__6_31 :26 __packed;
	} bit;
};

/* 4.5 Flash register map */
struct stm32f3x_flash {
	union __ef_acr acr;
	uint32_t keyr;
	uint32_t optkeyr;
	uint32_t sr;
	uint32_t cr;
	uint32_t ar;
	uint32_t rsvd;
	uint32_t obr;
	uint32_t wrpr;
};

#endif	/* _STM32F3X_FLASHREGISTERS_H_ */
