/*
 * Copyright (c) 2016 Intel Corporation.
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
#include <errno.h>

#include <device.h>
#if defined(CONFIG_IOAPIC) || defined(CONFIG_MVIC)
#include <ioapic.h>
#endif
#include <uart.h>
#include <power.h>

#include "qm_uart.h"
#include "qm_isr.h"
#include "clk.h"
#include "soc.h"
#include "qm_soc_regs.h"

#define IIR_IID_NO_INTERRUPT_PENDING 0x01

#define DIVISOR_LOW(baudrate) \
	((CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / (16 * baudrate)) & 0xFF)
#define DIVISOR_HIGH(baudrate) \
	(((CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / (16 * baudrate)) & 0xFF00) >> 8)

/* Convenient macro to get the controller instance. */
#define GET_CONTROLLER_INSTANCE(dev) \
	(((struct uart_qmsi_config_info *)dev->config->config_info)->instance)

struct uart_qmsi_config_info {
	qm_uart_t instance;
	clk_periph_t clock_gate;
	uint32_t baud_divisor;
	bool hw_fc;

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	uart_irq_config_func_t irq_config_func;
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */
};


static int uart_qmsi_init(struct device *dev);

#ifndef CONFIG_DEVICE_POWER_MANAGEMENT
struct uart_qmsi_drv_data {
	uart_irq_callback_t user_cb;
	uint8_t iir_cache;
};
#else
struct uart_context_t {
	uint32_t ier; /**< Interrupt Enable Register. */
	uint32_t dlh; /**< Divisor Latch High. */
	uint32_t dll; /**< Divisor Latch Low. */
	uint32_t lcr; /**< Line Control. */
	uint32_t mcr; /**< Modem Control. */
	uint32_t scr; /**< Scratchpad. */
	uint32_t htx; /**< Halt Transmission. */
	uint32_t dlf; /**< Divisor Latch Fraction. */
	uint32_t int_uart_mask; /**< Interrupt Mask. */
};

struct uart_qmsi_drv_data {
	uart_irq_callback_t user_cb;
	uint8_t iir_cache;
	struct uart_context_t ctx_save;
};

static int uart_suspend_device(struct device *dev, int pm_policy)
{
	if (device_busy_check(dev)) {
		return -EBUSY;
	}

	if (pm_policy == SYS_PM_DEEP_SLEEP) {
		struct uart_qmsi_config_info *config = dev->config->config_info;
		qm_uart_reg_t *const regs = QM_UART[config->instance];
		struct uart_qmsi_drv_data *drv_data = dev->driver_data;
		struct uart_context_t *const ctx_save = &drv_data->ctx_save;

		if (config->instance == QM_UART_0) {
			ctx_save->int_uart_mask = QM_SCSS_INT->int_uart_0_mask;
		} else {
			ctx_save->int_uart_mask = QM_SCSS_INT->int_uart_1_mask;
		}

		ctx_save->ier = regs->ier_dlh;
		ctx_save->lcr = regs->lcr;
		ctx_save->mcr = regs->mcr;
		ctx_save->scr = regs->scr;
		ctx_save->htx = regs->htx;
		ctx_save->dlf = regs->dlf;

		/* When DLAB is set, DLL and DLH registers can be accessed. */
		regs->lcr |= QM_UART_LCR_DLAB;
		ctx_save->dlh = regs->ier_dlh;
		ctx_save->dll = regs->rbr_thr_dll;
		regs->lcr &= ~QM_UART_LCR_DLAB;
	}

	return 0;
}

static int uart_resume_device(struct device *dev, int pm_policy)
{
	if (pm_policy == SYS_PM_DEEP_SLEEP) {
		struct uart_qmsi_config_info *config = dev->config->config_info;
		qm_uart_reg_t *const regs = QM_UART[config->instance];
		struct uart_qmsi_drv_data *drv_data = dev->driver_data;
		struct uart_context_t *const ctx_save = &drv_data->ctx_save;

		clk_periph_enable(config->clock_gate);

		if (config->instance == QM_UART_0) {
			QM_SCSS_INT->int_uart_0_mask = ctx_save->int_uart_mask;
		} else {
			QM_SCSS_INT->int_uart_1_mask = ctx_save->int_uart_mask;
		}

		/* When DLAB is set, DLL and DLH registers can be accessed. */
		regs->lcr |= QM_UART_LCR_DLAB;
		regs->ier_dlh = ctx_save->dlh;
		regs->rbr_thr_dll = ctx_save->dll;
		regs->lcr &= ~QM_UART_LCR_DLAB;

		regs->ier_dlh = ctx_save->ier;
		regs->lcr = ctx_save->lcr;
		regs->mcr = ctx_save->mcr;
		regs->scr = ctx_save->scr;
		regs->htx = ctx_save->htx;
		regs->dlf = ctx_save->dlf;

		/*
		 * FIFO control register cannot be read back,
		 * default config is applied for this register.
		 * Application will need to restore its own parameters.
		 */
		regs->iir_fcr =
			(QM_UART_FCR_FIFOE | QM_UART_FCR_RFIFOR |
			 QM_UART_FCR_XFIFOR |
			 QM_UART_FCR_DEFAULT_TX_RX_THRESHOLD);
	}

	return 0;
}
#endif /* CONFIG_DEVICE_POWER_MANAGEMENT */

DEFINE_DEVICE_PM_OPS(uart, uart_suspend_device, uart_resume_device);

#ifdef CONFIG_UART_QMSI_0
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void irq_config_func_0(struct device *dev);
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

static struct uart_qmsi_config_info config_info_0 = {
	.instance = QM_UART_0,
	.clock_gate = CLK_PERIPH_UARTA_REGISTER | CLK_PERIPH_CLK,
	.baud_divisor = QM_UART_CFG_BAUD_DL_PACK(
				DIVISOR_HIGH(CONFIG_UART_QMSI_0_BAUDRATE),
				DIVISOR_LOW(CONFIG_UART_QMSI_0_BAUDRATE),
				0),
#ifdef CONFIG_UART_QMSI_0_HW_FC
	.hw_fc = true,
#endif

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.irq_config_func = irq_config_func_0,
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */
};

static struct uart_qmsi_drv_data drv_data_0;

DEVICE_INIT_PM(uart_0, CONFIG_UART_QMSI_0_NAME, &uart_qmsi_init,
	       DEVICE_PM_OPS_GET(uart), &drv_data_0, &config_info_0, PRIMARY,
	       CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
#endif /* CONFIG_UART_QMSI_0 */

#ifdef CONFIG_UART_QMSI_1
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void irq_config_func_1(struct device *dev);
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

static struct uart_qmsi_config_info config_info_1 = {
	.instance = QM_UART_1,
	.clock_gate = CLK_PERIPH_UARTB_REGISTER | CLK_PERIPH_CLK,
	.baud_divisor = QM_UART_CFG_BAUD_DL_PACK(
				DIVISOR_HIGH(CONFIG_UART_QMSI_1_BAUDRATE),
				DIVISOR_LOW(CONFIG_UART_QMSI_1_BAUDRATE),
				0),
#ifdef CONFIG_UART_QMSI_1_HW_FC
	.hw_fc = true,
#endif

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.irq_config_func = irq_config_func_1,
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */
};

static struct uart_qmsi_drv_data drv_data_1;

DEVICE_INIT_PM(uart_1, CONFIG_UART_QMSI_1_NAME, &uart_qmsi_init,
	       DEVICE_PM_OPS_GET(uart), &drv_data_1, &config_info_1, PRIMARY,
	       CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
#endif /* CONFIG_UART_QMSI_1 */

static int uart_qmsi_poll_in(struct device *dev, unsigned char *data)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	qm_uart_status_t status;

	qm_uart_get_status(instance, &status);

	/* In order to check if there is any data to read from UART
	 * controller we should check if the QM_UART_RX_BUSY bit from
	 * 'status' is not set. This bit is set only if there is any
	 * pending character to read.
	 */
	if (!(status & QM_UART_RX_BUSY))
		return -1;

	qm_uart_read(instance, data, NULL);
	return 0;
}

static unsigned char uart_qmsi_poll_out(struct device *dev,
					unsigned char data)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	qm_uart_write(instance, data);
	return data;
}

static int uart_qmsi_err_check(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	qm_uart_status_t status;

	/* QMSI and Zephyr use the same bits to represent UART errors
	 * so we don't need to translate each error bit from QMSI API
	 * to Zephyr API.
	 */
	qm_uart_get_status(instance, &status);
	return (status & QM_UART_LSR_ERROR_BITS);
}

#if CONFIG_UART_INTERRUPT_DRIVEN
static bool is_tx_fifo_full(qm_uart_t instance)
{
	return !!(QM_UART[instance]->lsr & QM_UART_LSR_THRE);
}

static int uart_qmsi_fifo_fill(struct device *dev, const uint8_t *tx_data,
				  int size)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	int i;

	device_busy_set(dev);

	for (i = 0; i < size && !is_tx_fifo_full(instance); i++) {
		QM_UART[instance]->rbr_thr_dll = tx_data[i];
	}

	return i;
}

static bool is_data_ready(qm_uart_t instance)
{
	return QM_UART[instance]->lsr & QM_UART_LSR_DR;
}

static int uart_qmsi_fifo_read(struct device *dev, uint8_t *rx_data,
				  const int size)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	int i;

	device_busy_set(dev);

	for (i = 0; i < size && is_data_ready(instance); i++) {
		rx_data[i] = QM_UART[instance]->rbr_thr_dll;
	}

	return i;
}

static void uart_qmsi_irq_tx_enable(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	QM_UART[instance]->ier_dlh |= QM_UART_IER_ETBEI;
}

static void uart_qmsi_irq_tx_disable(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	QM_UART[instance]->ier_dlh &= ~QM_UART_IER_ETBEI;
}

static int uart_qmsi_irq_tx_ready(struct device *dev)
{
	struct uart_qmsi_drv_data *drv_data = dev->driver_data;
	uint32_t id = (drv_data->iir_cache & QM_UART_IIR_IID_MASK);

	return id == QM_UART_IIR_THR_EMPTY;
}

static int uart_qmsi_irq_tx_empty(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	const uint32_t mask = (QM_UART_LSR_TEMT | QM_UART_LSR_THRE);

	return (QM_UART[instance]->lsr & mask) == mask;
}

static void uart_qmsi_irq_rx_enable(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	QM_UART[instance]->ier_dlh |= QM_UART_IER_ERBFI;
}

static void uart_qmsi_irq_rx_disable(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	QM_UART[instance]->ier_dlh &= ~QM_UART_IER_ERBFI;
}

static int uart_qmsi_irq_rx_ready(struct device *dev)
{
	struct uart_qmsi_drv_data *drv_data = dev->driver_data;
	uint32_t id = (drv_data->iir_cache & QM_UART_IIR_IID_MASK);

	return (id == QM_UART_IIR_RECV_DATA_AVAIL) ||
	       (id == QM_UART_IIR_CHAR_TIMEOUT);
}

static void uart_qmsi_irq_err_enable(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	QM_UART[instance]->ier_dlh |= QM_UART_IER_ELSI;
}

static void uart_qmsi_irq_err_disable(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);

	QM_UART[instance]->ier_dlh &= ~QM_UART_IER_ELSI;
}

static int uart_qmsi_irq_is_pending(struct device *dev)
{
	struct uart_qmsi_drv_data *drv_data = dev->driver_data;
	uint32_t id = (drv_data->iir_cache & QM_UART_IIR_IID_MASK);

	return !(id == IIR_IID_NO_INTERRUPT_PENDING);
}

static int uart_qmsi_irq_update(struct device *dev)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	struct uart_qmsi_drv_data *drv_data = dev->driver_data;

	drv_data->iir_cache = QM_UART[instance]->iir_fcr;
	return 1;
}

static void uart_qmsi_irq_callback_set(struct device *dev,
				       uart_irq_callback_t cb)
{
	struct uart_qmsi_drv_data *drv_data = dev->driver_data;

	drv_data->user_cb = cb;
}

static void uart_qmsi_isr(void *arg)
{
	struct device *dev = arg;
	struct uart_qmsi_drv_data *drv_data = dev->driver_data;

	if (drv_data->user_cb)
		drv_data->user_cb(dev);

	device_busy_clear(dev);
}

#ifdef CONFIG_UART_QMSI_0
static void irq_config_func_0(struct device *dev)
{
	IRQ_CONNECT(QM_IRQ_UART_0, CONFIG_UART_QMSI_0_IRQ_PRI,
		    uart_qmsi_isr, DEVICE_GET(uart_0),
		    UART_IRQ_FLAGS);
	irq_enable(QM_IRQ_UART_0);
	QM_SCSS_INT->int_uart_0_mask &= ~BIT(0);
}
#endif /* CONFIG_UART_QMSI_0 */

#ifdef CONFIG_UART_QMSI_1
static void irq_config_func_1(struct device *dev)
{
	IRQ_CONNECT(QM_IRQ_UART_1, CONFIG_UART_QMSI_1_IRQ_PRI,
		    uart_qmsi_isr, DEVICE_GET(uart_1),
		    UART_IRQ_FLAGS);
	irq_enable(QM_IRQ_UART_1);
	QM_SCSS_INT->int_uart_1_mask &= ~BIT(0);
}
#endif /* CONFIG_UART_QMSI_1 */
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

#ifdef CONFIG_UART_LINE_CTRL
static int uart_qmsi_line_ctrl_set(struct device *dev, uint32_t ctrl, uint32_t val)
{
	qm_uart_t instance = GET_CONTROLLER_INSTANCE(dev);
	qm_uart_config_t cfg;

	switch (ctrl) {
	case LINE_CTRL_BAUD_RATE:
		cfg.line_control = QM_UART[instance]->lcr;
		cfg.baud_divisor = QM_UART_CFG_BAUD_DL_PACK(DIVISOR_HIGH(val),
							    DIVISOR_LOW(val), 0);
		cfg.hw_fc = QM_UART[instance]->mcr & QM_UART_MCR_AFCE;
		cfg.int_en = false;
		qm_uart_set_config(instance, &cfg);
		break;
	default:
		return -ENODEV;
	}

	return 0;
}
#endif /* CONFIG_UART_LINE_CTRL */

#ifdef CONFIG_UART_DRV_CMD
static int uart_qmsi_drv_cmd(struct device *dev, uint32_t cmd, uint32_t p)
{
	return -ENODEV;
}
#endif /* CONFIG_UART_DRV_CMD */

static struct uart_driver_api api = {
	.poll_in = uart_qmsi_poll_in,
	.poll_out = uart_qmsi_poll_out,
	.err_check = uart_qmsi_err_check,

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.fifo_fill = uart_qmsi_fifo_fill,
	.fifo_read = uart_qmsi_fifo_read,
	.irq_tx_enable = uart_qmsi_irq_tx_enable,
	.irq_tx_disable = uart_qmsi_irq_tx_disable,
	.irq_tx_ready = uart_qmsi_irq_tx_ready,
	.irq_tx_empty = uart_qmsi_irq_tx_empty,
	.irq_rx_enable = uart_qmsi_irq_rx_enable,
	.irq_rx_disable = uart_qmsi_irq_rx_disable,
	.irq_rx_ready = uart_qmsi_irq_rx_ready,
	.irq_err_enable = uart_qmsi_irq_err_enable,
	.irq_err_disable = uart_qmsi_irq_err_disable,
	.irq_is_pending = uart_qmsi_irq_is_pending,
	.irq_update = uart_qmsi_irq_update,
	.irq_callback_set = uart_qmsi_irq_callback_set,
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

#ifdef CONFIG_UART_LINE_CTRL
	.line_ctrl_set = uart_qmsi_line_ctrl_set,
#endif /* CONFIG_UART_LINE_CTRL */

#ifdef CONFIG_UART_DRV_CMD
	.drv_cmd = uart_qmsi_drv_cmd,
#endif /* CONFIG_UART_DRV_CMD */
};

static int uart_qmsi_init(struct device *dev)
{
	struct uart_qmsi_config_info *config = dev->config->config_info;
	qm_uart_config_t cfg;

	cfg.line_control = QM_UART_LC_8N1;
	cfg.baud_divisor = config->baud_divisor;
	cfg.hw_fc = config->hw_fc;
	cfg.int_en = false;

	clk_periph_enable(config->clock_gate);

	qm_uart_set_config(config->instance, &cfg);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	config->irq_config_func(dev);
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

	dev->driver_api = &api;

	return 0;
}
