/*
 * Copyright (c) 2016 Intel Corporation
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

#include <device.h>
#include <gpio.h>
#include <misc/util.h>
#include <nanokernel.h>
#include <sensor.h>

#include "sensor_bmc150_magn.h"

extern struct bmc150_magn_data bmc150_magn_data;

int bmc150_magn_trigger_set(struct device *dev,
			    const struct sensor_trigger *trig,
			    sensor_trigger_handler_t handler)
{
	struct bmc150_magn_data *data = dev->driver_data;
	const struct bmc150_magn_config * const config =
					dev->config->config_info;
	uint8_t state;

#if defined(CONFIG_BMC150_MAGN_TRIGGER_DRDY)
	if (trig->type == SENSOR_TRIG_DATA_READY) {
		gpio_pin_disable_callback(data->gpio_drdy,
					config->gpio_drdy_int_pin);

		state = 0;
		if (handler) {
			state = 1;
		}

		data->handler_drdy = handler;
		data->trigger_drdy = *trig;

		if (i2c_reg_update_byte(data->i2c_master,
					config->i2c_slave_addr,
					BMC150_MAGN_REG_INT_DRDY,
					BMC150_MAGN_MASK_DRDY_EN,
					state << BMC150_MAGN_SHIFT_DRDY_EN)
					< 0) {
			SYS_LOG_DBG("failed to set DRDY interrupt");
			return -EIO;
		}

		gpio_pin_enable_callback(data->gpio_drdy,
					config->gpio_drdy_int_pin);
	}
#endif

	return 0;
}

static void bmc150_magn_gpio_drdy_callback(struct device *dev,
					   struct gpio_callback *cb,
					   uint32_t pins)
{
	struct bmc150_magn_data *data =
		CONTAINER_OF(cb, struct bmc150_magn_data, gpio_cb);
	const struct bmc150_magn_config * const config =
		data->dev->config->config_info;

	ARG_UNUSED(pins);

	gpio_pin_disable_callback(dev, config->gpio_drdy_int_pin);

	nano_isr_sem_give(&data->sem);
}

static void bmc150_magn_fiber_main(int arg1, int gpio_pin)
{
	struct device *dev = (struct device *) arg1;
	struct bmc150_magn_data *data = dev->driver_data;
	struct bmc150_magn_config *config = dev->config->config_info;
	uint8_t reg_val;

	while (1) {
		nano_fiber_sem_take(&data->sem, TICKS_UNLIMITED);

		while (i2c_reg_read_byte(data->i2c_master,
					 config->i2c_slave_addr,
					 BMC150_MAGN_REG_INT_STATUS,
					 &reg_val) < 0) {
			SYS_LOG_DBG("failed to clear data ready interrupt");
		}

		if (data->handler_drdy) {
			data->handler_drdy(dev, &data->trigger_drdy);
		}

		gpio_pin_enable_callback(data->gpio_drdy, gpio_pin);
	}
}

static int bmc150_magn_set_drdy_polarity(struct device *dev, int state)
{
	struct bmc150_magn_data *data = dev->driver_data;
	struct bmc150_magn_config *config = dev->config->config_info;

	if (state) {
		state = 1;
	}

	return i2c_reg_update_byte(data->i2c_master, config->i2c_slave_addr,
				   BMC150_MAGN_REG_INT_DRDY,
				   BMC150_MAGN_MASK_DRDY_DR_POLARITY,
				   state << BMC150_MAGN_SHIFT_DRDY_DR_POLARITY);
}

int bmc150_magn_init_interrupt(struct device *dev)
{
	const struct bmc150_magn_config * const config =
					  dev->config->config_info;
	struct bmc150_magn_data *data = dev->driver_data;

#if defined(CONFIG_BMC150_MAGN_TRIGGER_DRDY)
	if (bmc150_magn_set_drdy_polarity(dev, 0) < 0) {
		SYS_LOG_DBG("failed to set DR polarity");
		return -EIO;
	}

	if (i2c_reg_update_byte(data->i2c_master, config->i2c_slave_addr,
				BMC150_MAGN_REG_INT_DRDY,
				BMC150_MAGN_MASK_DRDY_EN,
				0 << BMC150_MAGN_SHIFT_DRDY_EN) < 0) {
		SYS_LOG_DBG("failed to set data ready interrupt enabled bit");
		return -EIO;
	}
#endif

	data->handler_drdy = NULL;

	nano_sem_init(&data->sem);

	task_fiber_start(data->fiber_stack,
			 CONFIG_BMC150_MAGN_TRIGGER_FIBER_STACK,
			 bmc150_magn_fiber_main, (int) dev,
			 config->gpio_drdy_int_pin, 10, 0);

	data->gpio_drdy = device_get_binding(config->gpio_drdy_dev_name);
	if (!data->gpio_drdy) {
		SYS_LOG_DBG("gpio controller %s not found",
			    config->gpio_drdy_dev_name);
		return -EINVAL;
	}

	gpio_pin_configure(data->gpio_drdy, config->gpio_drdy_int_pin,
			   GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE |
			   GPIO_INT_ACTIVE_LOW | GPIO_INT_DEBOUNCE);

	gpio_init_callback(&data->gpio_cb,
			   bmc150_magn_gpio_drdy_callback,
			   BIT(config->gpio_drdy_int_pin));

	if (gpio_add_callback(data->gpio_drdy, &data->gpio_cb) < 0) {
		SYS_LOG_DBG("failed to set gpio callback");
		return -EIO;
	}

	data->dev = dev;

	return 0;
}
