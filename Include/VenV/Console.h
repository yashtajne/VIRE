#ifndef VenVConsole_h
#define VenVConsole_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Console Device
 *
 * Provides:
 * - Simple character-based console I/O
 * - Guest OS can use as system console
 * - Host connects to stdin/stdout or terminal
 * - Memory-mapped interface with optional buffer
 */

#include "../Interface/Core/Int.h"
#include "../Interface/Core/Error.h"
#include "Device.h"

/*---- Console Configuration ----*/

#define VENV_CONSOLE_MMIO_SIZE  0x1000      /* 4KB MMIO region */
#define VENV_CONSOLE_BUFFER_SIZE 4096       /* Input buffer size */

/*---- Console Registers (MMIO offsets) ----*/

#define VENV_CONSOLE_DATA       0x00        /* Data register (read/write) */
#define VENV_CONSOLE_STATUS     0x08        /* Status register */

/* Status register bits */
#define VENV_CONSOLE_STATUS_RX_READY  0x01  /* Receive data ready */
#define VENV_CONSOLE_STATUS_TX_READY  0x02  /* Transmit buffer empty */
#define VENV_CONSOLE_STATUS_IRQ_EN    0x04  /* Interrupt enable */

/*---- Console State ----*/

struct VenVConsoleState
{
    venv_device_t base;         /* Base device structure */

    uint8_t* rx_buffer;         /* Input buffer */
    uint64_t rx_head;           /* Read index */
    uint64_t rx_tail;           /* Write index */
    uint64_t rx_size;           /* Buffer size */

    uint64_t status;            /* Status register */
    uint64_t control;           /* Control register */

    /* Host-side file descriptors for I/O */
    int host_stdin;
    int host_stdout;
};

typedef struct VenVConsoleState venv_console_t;

/*---- Console Operations ----*/

/* Initialize console device */
err_t venv_console_init(venv_console_t* console);

/* Get console device structure */
venv_device_t* venv_console_get_device(venv_console_t* console);

/* Write a character to console (from guest) */
err_t venv_console_putchar(venv_console_t* console, char_t ch);

/* Read a character from console (by guest) */
err_t venv_console_getchar(venv_console_t* console, char_t* out_ch);

/* Push input from host to console buffer */
err_t venv_console_push_input(venv_console_t* console, const char* data, uint64_t len);

/* Check if console has pending interrupt */
boolean venv_console_irq_pending(venv_console_t* console);

/* Acknowledge console interrupt */
err_t venv_console_ack_irq(venv_console_t* console);

#endif /* VenVConsole_h */
