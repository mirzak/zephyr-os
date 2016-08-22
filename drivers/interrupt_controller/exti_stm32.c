/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
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
 * @brief Driver for External interrupt/event controller in STM32 MCUs
 *
 * Based on reference manual:
 *   STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx
 *   advanced ARM ® -based 32-bit MCUs
 *
 * Chapter 10.2: External interrupt/event controller (EXTI)
 *
 */
#include <device.h>
#include <soc.h>
#include <misc/__assert.h>
#include "exti_stm32.h"

/* 10.3.7 EXTI register map */
struct stm32_exti {
	/* EXTI_IMR */
	uint32_t imr;
	/* EXTI_EMR */
	uint32_t emr;
	/* EXTI_RTSR */
	uint32_t rtsr;
	/* EXTI_FTSR */
	uint32_t ftsr;
	/* EXTI_SWIER */
	uint32_t swier;
	/* EXTI_PR */
	uint32_t pr;
};

/* wrapper for user callback */
struct __exti_cb {
	stm32_exti_callback_t cb;
	void *data;
};

#ifdef CONFIG_SOC_SERIES_STM32F1X
#define EXTI_LINES 19
#elif CONFIG_SOC_SERIES_STM32F3X
#define EXTI_LINES 21
#endif

/* driver data */
struct stm32_exti_data {
	/* per-line callbacks */
	struct __exti_cb cb[EXTI_LINES];
};


#define AS_EXTI(__base_addr)			\
	((struct stm32_exti *)(__base_addr))

void stm32_exti_enable(struct device *dev, int line)
{
	volatile struct stm32_exti *exti = AS_EXTI(EXTI_BASE);
	int irqnum = -1;

	ARG_UNUSED(dev);

	exti->imr |= 1 << line;

#ifdef CONFIG_SOC_SERIES_STM32F1X
	if (line >= 5 && line <= 9) {
		irqnum = STM32F1_IRQ_EXTI9_5;
	} else if (line >= 10 && line <= 15) {
		irqnum = STM32F1_IRQ_EXTI15_10;
	} else {
		/* pins 0..4 are mapped to EXTI0.. EXTI4 */
		irqnum = STM32F1_IRQ_EXTI0 + line;
	}
#elif CONFIG_SOC_SERIES_STM32F3X
	if (line >= 5 && line <= 9) {
		irqnum = STM32F3_IRQ_EXTI9_5;
	} else if (line >= 10 && line <= 15) {
		irqnum = STM32F3_IRQ_EXTI15_10;
	} else if (line >=0 && line <=4) {
		/* pins 0..4 are mapped to EXTI0.. EXTI4 */
		irqnum = STM32F3_IRQ_EXTI0 + line;
	} else switch (line) {
	case 16:
	irqnum = STM32F3_IRQ_EXTI16;
	break;
	case 18:
	irqnum = STM32F3_IRQ_EXTI18;
	break;
	case 19:
	irqnum = STM32F3_IRQ_EXTI19;
	break;
	case 20:
	irqnum = STM32F3_IRQ_EXTI20;
	break;
	case 21:
	case 22:
	irqnum = STM32F3_IRQ_EXTI_21_22_29;
	break;
	case 23:
	irqnum = STM32F3_IRQ_EXTI23;
	break;
	case 24:
	irqnum = STM32F3_IRQ_EXTI24;
	break;
	case 25:
	irqnum = STM32F3_IRQ_EXTI25;
	break;
	case 26:
	irqnum = STM32F3_IRQ_EXTI26;
	break;
	case 28:
	irqnum = STM32F3_IRQ_EXTI28;
	break;
	case 29:
	irqnum = STM32F3_IRQ_EXTI_21_22_29;
	break;
	case 30:
	case 31:
	case 32:
	irqnum = STM32F3_IRQ_EXTI_30_31_32;
	case 34:
	irqnum = STM32F3_IRQ_EXTI34;
	break;
	case 35:
	irqnum = STM32F3_IRQ_EXTI35;
	break;
	}
#else
#error "unsupported SOC series specified";
#endif

	__ASSERT(irqnum < 0,
		"EXTI unsupported irqnum specified: %d", irqnum);

	irq_enable(irqnum);
}

void stm32_exti_disable(struct device *dev, int line)
{
	volatile struct stm32_exti *exti = AS_EXTI(EXTI_BASE);

	ARG_UNUSED(dev);

	exti->imr &= ~(1 << line);
}

/**
 * @brief check if interrupt is pending
 *
 * @param line line number
 */
static inline int stm32_exti_is_pending(int line)
{
	volatile struct stm32_exti *exti = AS_EXTI(EXTI_BASE);

	return (exti->pr & (1 << line)) ? 1 : 0;
}

/**
 * @brief clear pending interrupt bit
 *
 * @param line line number
 */
static inline void stm32_exti_clear_pending(int line)
{
	volatile struct stm32_exti *exti = AS_EXTI(EXTI_BASE);

	exti->pr |= 1 << line;
}

void stm32_exti_trigger(struct device *dev, int line, int trigger)
{
	volatile struct stm32_exti *exti = AS_EXTI(EXTI_BASE);

	ARG_UNUSED(dev);

	if (trigger & STM32_EXTI_TRIG_RISING) {
		exti->rtsr |= 1 << line;
	}

	if (trigger & STM32_EXTI_TRIG_FALLING) {
		exti->ftsr |= 1 << line;
	}
}

void stm32_exti_set_callback(struct device *dev, int line,
			stm32_exti_callback_t cb, void *arg)
{
	struct stm32_exti_data *data = dev->driver_data;

	__ASSERT(data->cb[line].cb == NULL,
		"EXTI %d callback already registered", line);

	data->cb[line].cb = cb;
	data->cb[line].data = arg;
}

void stm32_exti_unset_callback(struct device *dev, int line)
{
	struct stm32_exti_data *data = dev->driver_data;

	data->cb[line].cb = NULL;
	data->cb[line].data = NULL;
}

/**
 * @brief EXTI ISR handler
 *
 * Check EXTI lines in range @min @max for pending interrupts
 *
 * @param arg isr argument
 * @parram min low end of EXTI# range
 * @parram max low end of EXTI# range
 */
static void __stm32_exti_isr(int min, int max, void *arg)
{
	struct device *dev = arg;
	struct stm32_exti_data *data = dev->driver_data;
	int line;

	/* see which bits are set */
	for (line = min; line < max; line++) {
		/* check if interrupt is pending */
		if (stm32_exti_is_pending(line)) {
			/* clear pending interrupt */
			stm32_exti_clear_pending(line);

			/* run callback only if one is registered */
			if (!data->cb[line].cb) {
				continue;
			}

			data->cb[line].cb(line, data->cb[line].data);
		}
	}
}

static inline void __stm32_exti_isr_0(void *arg)
{
	__stm32_exti_isr(0, 1, arg);
}

static inline void __stm32_exti_isr_1(void *arg)
{
	__stm32_exti_isr(1, 2, arg);
}

static inline void __stm32_exti_isr_2(void *arg)
{
	__stm32_exti_isr(2, 3, arg);
}

static inline void __stm32_exti_isr_3(void *arg)
{
	__stm32_exti_isr(3, 4, arg);
}

static inline void __stm32_exti_isr_4(void *arg)
{
	__stm32_exti_isr(4, 5, arg);
}

static inline void __stm32_exti_isr_9_5(void *arg)
{
	__stm32_exti_isr(5, 10, arg);
}

static inline void __stm32_exti_isr_15_10(void *arg)
{
	__stm32_exti_isr(10, 16, arg);
}

#ifdef CONFIG_SOC_SERIES_STM32F3X
static inline void __stm32_exti_isr_16(void *arg)
{
	__stm32_exti_isr(16, 17, arg);
}

static inline void __stm32_exti_isr_18(void *arg)
{
	__stm32_exti_isr(18, 19, arg);
}

static inline void __stm32_exti_isr_19(void *arg)
{
	__stm32_exti_isr(19, 20, arg);
}

static inline void __stm32_exti_isr_20(void *arg)
{
	__stm32_exti_isr(20, 21, arg);
}

#warning "exti STM32F3 is this really correct?"
static inline void __stm32_exti_isr_21_22_29(void *arg)
{
	__stm32_exti_isr(21, 30, arg);
}

static inline void __stm32_exti_isr_23(void *arg)
{
	__stm32_exti_isr(23, 24, arg);
}

static inline void __stm32_exti_isr_24(void *arg)
{
	__stm32_exti_isr(24, 25, arg);
}

static inline void __stm32_exti_isr_25(void *arg)
{
	__stm32_exti_isr(25, 26, arg);
}

static inline void __stm32_exti_isr_26(void *arg)
{
	__stm32_exti_isr(26, 27, arg);
}

static inline void __stm32_exti_isr_28(void *arg)
{
	__stm32_exti_isr(28, 29, arg);
}

static inline void __stm32_exti_isr_30_31_32(void *arg)
{
	__stm32_exti_isr(30, 33, arg);
}

static inline void __stm32_exti_isr_33(void *arg)
{
	__stm32_exti_isr(33, 34, arg);
}

static inline void __stm32_exti_isr_34(void *arg)
{
	__stm32_exti_isr(34, 34, arg);
}

static inline void __stm32_exti_isr_35(void *arg)
{
	__stm32_exti_isr(35, 36, arg);
}
#endif

static void __stm32_exti_connect_irqs(struct device *dev);

/**
 * @brief initialize EXTI device driver
 */
static int stm32_exti_init(struct device *dev)
{
	__stm32_exti_connect_irqs(dev);

	return 0;
}

static struct stm32_exti_data exti_data;
DEVICE_INIT(exti_stm32, STM32_EXTI_NAME, stm32_exti_init,
	    &exti_data, NULL,
	    PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

/**
 * @brief connect all interrupts
 */
static void __stm32_exti_connect_irqs(struct device *dev)
{

#ifdef CONFIG_SOC_SERIES_STM32F1X
IRQ_CONNECT(STM32F1_IRQ_EXTI0,
	CONFIG_EXTI_STM32_EXTI0_IRQ_PRI,
	__stm32_exti_isr_0, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F1_IRQ_EXTI1,
	CONFIG_EXTI_STM32_EXTI1_IRQ_PRI,
	__stm32_exti_isr_1, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F1_IRQ_EXTI2,
	CONFIG_EXTI_STM32_EXTI2_IRQ_PRI,
	__stm32_exti_isr_2, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F1_IRQ_EXTI3,
	CONFIG_EXTI_STM32_EXTI3_IRQ_PRI,
	__stm32_exti_isr_3, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F1_IRQ_EXTI4,
	CONFIG_EXTI_STM32_EXTI4_IRQ_PRI,
	__stm32_exti_isr_4, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F1_IRQ_EXTI9_5,
	CONFIG_EXTI_STM32_EXTI9_5_IRQ_PRI,
	__stm32_exti_isr_9_5, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F1_IRQ_EXTI15_10,
	CONFIG_EXTI_STM32_EXTI15_10_IRQ_PRI,
	__stm32_exti_isr_15_10, DEVICE_GET(exti_stm32),
	0);
#endif

#ifdef CONFIG_SOC_SERIES_STM32F3X
#warning "TODO: fix priority settings"
IRQ_CONNECT(STM32F3_IRQ_EXTI0,
	CONFIG_EXTI_STM32_EXTI0_IRQ_PRI,
	__stm32_exti_isr_0, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI1,
	CONFIG_EXTI_STM32_EXTI1_IRQ_PRI,
	__stm32_exti_isr_1, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI2_TS,
	CONFIG_EXTI_STM32_EXTI2_IRQ_PRI,
	__stm32_exti_isr_2, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI3,
	CONFIG_EXTI_STM32_EXTI3_IRQ_PRI,
	__stm32_exti_isr_3, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI4,
	CONFIG_EXTI_STM32_EXTI4_IRQ_PRI,
	__stm32_exti_isr_4, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI9_5,
	CONFIG_EXTI_STM32_EXTI9_5_IRQ_PRI,
	__stm32_exti_isr_9_5, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI15_10,
	CONFIG_EXTI_STM32_EXTI15_10_IRQ_PRI,
	__stm32_exti_isr_15_10, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI16,
	0,
	__stm32_exti_isr_16, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI18,
	0,
	__stm32_exti_isr_18, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI19,
	0,
	__stm32_exti_isr_19, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI20,
	0,
	__stm32_exti_isr_20, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI_21_22_29,
	0,
	__stm32_exti_isr_21_22_29, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI23,
	0,
	__stm32_exti_isr_23, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI24,
	0,
	__stm32_exti_isr_24, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI25,
	0,
	__stm32_exti_isr_25, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI26,
	0,
	__stm32_exti_isr_26, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI28,
	0,
	__stm32_exti_isr_28, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI_30_31_32,
	0,
	__stm32_exti_isr_30_31_32, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI33,
	0,
	__stm32_exti_isr_33, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI34,
	0,
	__stm32_exti_isr_34, DEVICE_GET(exti_stm32),
	0);
IRQ_CONNECT(STM32F3_IRQ_EXTI35,
	0,
	__stm32_exti_isr_35, DEVICE_GET(exti_stm32),
	0);
#endif
}


