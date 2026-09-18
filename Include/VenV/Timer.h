#ifndef VenVTimer_h
#define VenVTimer_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Timer Device
 *
 * Provides:
 * - Programmable timer that generates interrupts
 * - Used by guest OS for scheduling, timekeeping, sleeping
 * - Memory-mapped interface
 */

#include "../Interface/Core/Int.h"
#include "../Interface/Core/Error.h"
#include "Device.h"

/*---- Timer Configuration ----*/

#define VENV_TIMER_MMIO_SIZE    0x1000      /* 4KB MMIO region */
#define VENV_TIMER_FREQ         1000000     /* 1 MHz timer frequency */

/*---- Timer Registers (MMIO offsets) ----*/

#define VENV_TIMER_CTRL         0x00        /* Control register */
#define VENV_TIMER_PRESCALE     0x08        /* Prescaler value */
#define VENV_TIMER_COMPARE      0x10        /* Compare value */
#define VENV_TIMER_COUNTER      0x18        /* Current counter value */
#define VENV_TIMER_STATUS       0x20        /* Status/interrupt flag */

/* Control register bits */
#define VENV_TIMER_CTRL_ENABLE  0x01        /* Enable timer */
#define VENV_TIMER_CTRL_PERIODIC 0x02       /* Periodic mode */
#define VENV_TIMER_CTRL_IRQ_EN  0x04        /* Interrupt enable */

/*---- Timer State ----*/

struct VenVTimerState
{
    venv_device_t base;         /* Base device structure */

    uint64_t control;           /* Control register */
    uint64_t prescale;          /* Prescaler value */
    uint64_t compare;           /* Compare value for interrupt */
    uint64_t counter;           /* Current counter */
    uint64_t status;            /* Status register */

    uint64_t cycles;            /* Internal cycle counter */
};

typedef struct VenVTimerState venv_timer_t;

/*---- Timer Operations ----*/

/* Initialize timer device */
err_t venv_timer_init(venv_timer_t* timer);

/* Get timer device structure */
venv_device_t* venv_timer_get_device(venv_timer_t* timer);

/* Update timer state (called by VM loop) */
err_t venv_timer_update(venv_timer_t* timer, uint64_t cycles);

/* Check if timer interrupt is pending */
boolean venv_timer_irq_pending(venv_timer_t* timer);

/* Acknowledge timer interrupt */
err_t venv_timer_ack_irq(venv_timer_t* timer);

#endif /* VenVTimer_h */
