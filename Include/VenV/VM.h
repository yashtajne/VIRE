#ifndef VenVVM_h
#define VenVVM_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Machine / Environment Abstraction
 *
 * This is the main interface for creating and managing
 * virtual environments.
 */

#include "../Interface/Core/Int.h"
#include "../Interface/Core/Error.h"
#include "CPU.h"
#include "Memory.h"
#include "Device.h"
#include "Timer.h"
#include "Console.h"
#include "Block.h"
#include "Net.h"

/*---- VM Configuration ----*/

#define VENV_VM_NAME_MAX        64          /* Max environment name length */
#define VENV_VM_DEVICES_MAX     16          /* Max devices per VM */

/*---- MMIO Mapping ----*/

struct VenVMMIOMapping
{
    uint64_t base;              /* Base address */
    uint64_t size;              /* Size */
    venv_device_t* device;      /* Device mapped here */
    boolean is_mapped;
};

typedef struct VenVMMIOMapping venv_mmio_mapping_t;

/*---- VM State ----*/

struct VenVVMState
{
    /* Environment info */
    char name[VENV_VM_NAME_MAX];
    char config_path[256];      /* Path to config directory */
    char disk_path[256];        /* Path to disk image */

    /* CPU and memory */
    venv_cpu_t cpu;
    venv_memory_t memory;

    /* Devices */
    venv_timer_t timer;
    venv_console_t console;
    venv_block_t block;
    venv_net_t net;

    /* MMIO mappings */
    venv_mmio_mapping_t mmio[VENV_VM_DEVICES_MAX];
    uint64_t mmio_count;

    /* Execution state */
    boolean is_running;
    boolean is_paused;
    uint64_t cycles_executed;

    /* Interrupt controller state */
    uint64_t pending_irqs;

    /* Statistics */
    uint64_t total_cycles;
    uint64_t total_instructions;
    uint64_t exit_reason;       /* Last exit reason */
};

typedef struct VenVVMState venv_vm_t;

/*---- VM Lifecycle ----*/

/* Create a new VM with given name and configuration */
err_t venv_vm_create(venv_vm_t* vm, const char* name, uint64_t ram_size);

/* Destroy VM and free all resources */
void venv_vm_destroy(venv_vm_t* vm);

/* Initialize VM devices (called after create) */
err_t venv_vm_init_devices(venv_vm_t* vm, const char* disk_path);

/* Reset VM to initial state */
err_t venv_vm_reset(venv_vm_t* vm);

/* Start VM execution */
err_t venv_vm_start(venv_vm_t* vm);

/* Stop VM execution */
err_t venv_vm_stop(venv_vm_t* vm);

/* Pause VM execution */
err_t venv_vm_pause(venv_vm_t* vm);

/* Resume paused VM */
err_t venv_vm_resume(venv_vm_t* vm);

/*---- VM Execution ----*/

/* Run VM for N instructions (or until event) */
err_t venv_vm_run(venv_vm_t* vm, uint64_t max_instructions);

/* Run VM until halted or error */
err_t venv_vm_run_loop(venv_vm_t* vm);

/* Execute one instruction and handle devices */
err_t venv_vm_step(venv_vm_t* vm);

/*---- VM State Access ----*/

/* Get CPU state */
venv_cpu_t* venv_vm_get_cpu(venv_vm_t* vm);

/* Get memory context */
venv_memory_t* venv_vm_get_memory(venv_vm_t* vm);

/* Load binary into VM memory at specified address */
err_t venv_vm_load_binary(venv_vm_t* vm, const uint8_t* data, uint64_t size, uint64_t addr);

/* Load binary from file into VM memory */
err_t venv_vm_load_file(venv_vm_t* vm, const char* filename, uint64_t addr);

/* Set entry point (PC) */
err_t venv_vm_set_entry(venv_vm_t* vm, uint64_t entry_addr);

/*---- Console I/O ----*/

/* Push input to VM console */
err_t venv_vm_console_input(venv_vm_t* vm, const char* data, uint64_t len);

/* Read output from VM console */
err_t venv_vm_console_output(venv_vm_t* vm, char* buffer, uint64_t max_len, uint64_t* out_len);

/*---- Network ----*/

/* Send network packet to VM */
err_t venv_vm_net_receive(venv_vm_t* vm, const uint8_t* pkt, uint64_t len);

/* Get VM MAC address */
const uint8_t* venv_vm_get_mac(venv_vm_t* vm);

#endif /* VenVVM_h */
