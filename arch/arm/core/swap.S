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

/**
 * @file
 * @brief Thread context switching for ARM Cortex-M
 *
 * This module implements the routines necessary for thread context switching
 * on ARM Cortex-M3/M4 CPUs.
 */

#define _ASMLANGUAGE

#include <nano_private.h>
#include <offsets.h>
#include <toolchain.h>
#include <arch/cpu.h>

_ASM_FILE_PROLOGUE

GTEXT(_Swap)
GTEXT(__svc)
GTEXT(__pendsv)

GDATA(_nanokernel)

/**
 *
 * @brief PendSV exception handler, handling context switches
 *
 * The PendSV exception is the only execution context in the system that can
 * perform context switching. When an execution context finds out it has to
 * switch contexts, it pends the PendSV exception.
 *
 * When PendSV is pended, the decision that a context switch must happen has
 * already been taken. In other words, when __pendsv() runs, we *know* we have
 * to swap *something*.
 *
 * The scheduling algorithm is simple: schedule the head of the runnable fibers
 * list (_nanokernel.fiber). If there are no runnable fibers, then schedule the
 * task (_nanokernel.task). The _nanokernel.task field will never be NULL.
 */

SECTION_FUNC(TEXT, __pendsv)

    _GDB_STUB_EXC_ENTRY

#ifdef CONFIG_KERNEL_EVENT_LOGGER_CONTEXT_SWITCH
	/* Register the context switch */
	push {lr}
	bl _sys_k_event_logger_context_switch
	pop {lr}
#endif

    /* load _Nanokernel into r1 and current tTCS into r2 */
    ldr r1, =_nanokernel
    ldr r2, [r1, #__tNANO_current_OFFSET]

    /* addr of callee-saved regs in TCS in r0 */
    add r0, r2, #__tTCS_preempReg_OFFSET

    /* save callee-saved + psp in TCS */
    mrs ip, PSP
    stmia r0, {v1-v8, ip}

#ifdef CONFIG_FP_SHARING
    add r0, r2, #__tTCS_preemp_float_regs_OFFSET
    vstmia r0, {s16-s31}
#endif

    /*
     * Prepare to clear PendSV with interrupts unlocked, but
     * don't clear it yet. PendSV must not be cleared until
     * the new thread is context-switched in since all decisions
     * to pend PendSV have been taken with the current kernel
     * state and this is what we're handling currently.
     */
    ldr ip, =_SCS_ICSR
    ldr r3, =_SCS_ICSR_UNPENDSV

    /* protect the kernel state while we play with the thread lists */
    movs.n r0, #_EXC_IRQ_DEFAULT_PRIO
    msr BASEPRI, r0

    /* find out incoming thread (fiber or task) */

    /* is there a fiber ready ? */
    ldr r2, [r1, #__tNANO_fiber_OFFSET]
    cmp r2, #0

    /*
     * if so, remove fiber from list
     * else, the task is the thread we're switching in
     */
    itte ne
	ldrne.w r0, [r2, #__tTCS_link_OFFSET]   /* then */
	strne.w r0, [r1, #__tNANO_fiber_OFFSET] /* then */
	ldreq.w r2, [r1, #__tNANO_task_OFFSET]  /* else */

    /* r2 contains the new thread */
    ldr r0, [r2, #__tTCS_flags_OFFSET]
    str r0, [r1, #__tNANO_flags_OFFSET]
    str r2, [r1, #__tNANO_current_OFFSET]

    /*
     * Clear PendSV so that if another interrupt comes in and
     * decides, with the new kernel state baseed on the new thread
     * being context-switched in, that it needs to reschedules, it
     * will take, but that previously pended PendSVs do not take,
     * since they were based on the previous kernel state and this
     * has been handled.
     */

    /* _SCS_ICSR is still in ip and _SCS_ICSR_UNPENDSV in r3 */
    str r3, [ip, #0]

    /* restore BASEPRI for the incoming thread */
    ldr r0, [r2, #__tTCS_basepri_OFFSET]
    mov ip, #0
    str ip, [r2, #__tTCS_basepri_OFFSET]
    msr BASEPRI, r0

#ifdef CONFIG_FP_SHARING
    add r0, r2, #__tTCS_preemp_float_regs_OFFSET
    vldmia r0, {s16-s31}
#endif

    /* load callee-saved + psp from TCS */
    add r0, r2, #__tTCS_preempReg_OFFSET
    ldmia r0, {v1-v8, ip}
    msr PSP, ip

    _GDB_STUB_EXC_EXIT

    /* exc return */
    bx lr

/**
 *
 * @brief Service call handler
 *
 * The service call (svc) is only used in _Swap() to enter handler mode so we
 * can go through the PendSV exception to perform a context switch.
 *
 * @return N/A
 */

SECTION_FUNC(TEXT, __svc)

    _GDB_STUB_EXC_ENTRY

#if CONFIG_IRQ_OFFLOAD
    tst lr, #0x4    /* did we come from thread mode ? */
    ite eq  /* if zero (equal), came from handler mode */
        mrseq r0, MSP   /* handler mode, stack frame is on MSP */
        mrsne r0, PSP   /* thread mode, stack frame is on PSP */

    ldr r0, [r0, #24]   /* grab address of PC from stack frame */
    /* SVC is a two-byte instruction, point to it and read  encoding */
    ldrh r0, [r0, #-2]

   /*
    * grab service call number: if zero, it's a context switch;  if not,
    * it's an irq offload
    */
    ands r0, #0xff
    beq _context_switch

    push {lr}
    blx _irq_do_offload  /* call C routine which executes the offload */
    pop {lr}

    /* exception return is done in _IntExit(), including _GDB_STUB_EXC_EXIT */
    b _IntExit

BRANCH_LABEL(_context_switch);
#endif

    /*
     * Unlock interrupts:
     * - in a SVC call, so protected against context switches
     * - allow PendSV, since it's running at prio 0xff
     */
    eors.n r0, r0
    msr BASEPRI, r0

     /* set PENDSV bit, pending the PendSV exception */
    ldr r1, =_SCS_ICSR
    ldr r2, =_SCS_ICSR_PENDSV
    str r2, [r1, #0]

    _GDB_STUB_EXC_EXIT

    /* handler mode exit, to PendSV */
    bx lr

/**
 *
 * @brief Initiate a cooperative context switch
 *
 * The _Swap() routine is invoked by various nanokernel services to effect
 * a cooperative context context switch.  Prior to invoking _Swap(), the caller
 * disables interrupts via irq_lock() and the return 'key' is passed as a
 * parameter to _Swap().  The 'key' actually represents the BASEPRI register
 * prior to disabling interrupts via the BASEPRI mechanism.
 *
 * _Swap() itself does not do much.
 *
 * It simply stores the intlock key (the BASEPRI value) parameter into
 * current->basepri, and then triggers a service call exception (svc) to setup
 * the PendSV exception, which does the heavy lifting of context switching.

 * This is the only place we have to save BASEPRI since the other paths to
 * __pendsv all come from handling an interrupt, which means we know the
 * interrupts were not locked: in that case the BASEPRI value is 0.
 *
 * Given that _Swap() is called to effect a cooperative context switch,
 * only the caller-saved integer registers need to be saved in the TCS of the
 * outgoing thread. This is all performed by the hardware, which stores it in
 * its exception stack frame, created when handling the svc exception.
 *
 * @return may contain a return value setup by a call to fiberRtnValueSet()
 *
 * C function prototype:
 *
 * unsigned int _Swap (unsigned int basepri);
 *
 */

SECTION_FUNC(TEXT, _Swap)

    ldr r1, =_nanokernel
    ldr r2, [r1, #__tNANO_current_OFFSET]
    str r0, [r2, #__tTCS_basepri_OFFSET]

    svc #0

    /* r0 contains the return value if needed */
    bx lr
