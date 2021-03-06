/* taskDevInt.h - APIs for managing task IRQ objects */

/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
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

#ifndef TASK_IRQ_H
#define TASK_IRQ_H


/**
 * @brief Microkernel Interrupt Services
 * @defgroup microkernel_irq Microkernel Interrupt Services
 * @ingroup microkernel_services
 * @{
 */

#include <microkernel/base_api.h>
#include <toolchain/gcc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_VECTOR 0xFFFFFFFF

/**
 * @fn uint32_t task_irq_alloc(kirq_t irq_obj, uint32_t irq,
 *                             uint32_t priority, uint32_t flags)
 * @brief Register a task IRQ object.
 *
 * This routine connects a task IRQ object to a system interrupt based
 * upon the specified IRQ and priority values.
 *
 * IRQ allocation is done via the microkernel server fiber, making simultaneous
 * allocation requests single-threaded.
 *
 * @param irq_obj IRQ object identifier.
 * @param irq Request IRQ.
 * @param priority Requested interrupt priority.
 * @param flags IRQ flags.
 *
 * @return assigned interrupt vector if successful, INVALID_VECTOR if not
 */
uint32_t __deprecated task_irq_alloc(kirq_t irq_obj, uint32_t irq,
				     uint32_t priority, uint32_t flags);

/**
 *
 * @fn void task_irq_ack(kirq_t irq_obj)
 * @brief Re-enable a task IRQ object's interrupt.
 *
 * This re-enables the interrupt for a task IRQ object.
 *
 * @param irq_obj IRQ object identifier.
 *
 * @return N/A
 */
void __deprecated task_irq_ack(kirq_t irq_obj);

/**
 * @fn int task_irq_wait(kirq_t irq_obj, int32_t timeout)
 * @brief Wait for task IRQ to signal an interrupt.
 *
 * This routine waits up to @a timeout ticks for the IRQ object @a irq_obj
 * to signal an interrupt.
 *
 * @param irq_obj IRQ object identifier.
 * @param timeout Determines the action when task IRQ is not yet signaled.
 *  For TICKS_NONE, return immediately.
 *  For TICKS_UNLIMITED, wait as long as necessary.
 *  Otherwise, wait up to the specified number of ticks before timing out.
 *
 * @retval RC_OK Successfully detected signaled interrupt.
 * @retval RC_TIME Timed out while waiting for interrupt.
 * @retval RC_FAIL Failed to immediately detect signaled interrupt when
 * @a timeout = TICKS_NONE.
 * @sa TICKS_NONE, TICKS_UNLIMITED
 */
int __deprecated task_irq_wait(kirq_t irq_obj, int32_t timeout);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* TASK_IRQ_H */
