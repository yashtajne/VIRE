#ifndef VenVCPU_h
#define VenVCPU_h

/*
 * VenV - Virtual Environment Engine
 * Virtual CPU State and Core Emulation
 */

#include "../Core/Int.h"
#include "../Core/Error.h"
#include "ISA.h"

/*---- CPU State ----*/

struct VenVCPUState
{
    /* General-purpose registers (16 registers, 64-bit each) */
    uint64_t regs[VENV_REG_COUNT];
    
    /* Program counter */
    uint64_t pc;
    
    /* Status/flags register */
    struct
    {
        uint64_t ie  : 1;   /* Interrupt enable */
        uint64_t pie : 1;   /* Previous interrupt enable */
        uint64_t priv: 2;   /* Privilege level */
        uint64_t wp  : 1;   /* Write protect */
        uint64_t reserved: 59;
    } status;
    
    /* Trap/interrupt state */
    uint64_t cause;         /* Exception/interrupt cause */
    uint64_t value;         /* Exception value (e.g., faulting address) */
    uint64_t epc;           /* Exception program counter */
    
    /* Control and Status Registers (CSRs) */
    uint64_t csrs[256];     /* CSR space */
    
    /* Cycle counter (for timing) */
    uint64_t cycle;
    
    /* Instruction counter */
    uint64_t instret;
};

typedef struct VenVCPUState venv_cpu_t;

/*---- CPU Operations ----*/

/* Initialize CPU to reset state */
err_t venv_cpu_reset(venv_cpu_t* cpu);

/* Execute one instruction */
err_t venv_cpu_step(venv_cpu_t* cpu, void* memory_context);

/* Execute N instructions (returns when count reached or exception) */
err_t venv_cpu_run(venv_cpu_t* cpu, void* memory_context, uint64_t count);

/* Raise an exception/interrupt */
err_t venv_cpu_raise_exception(venv_cpu_t* cpu, uint64_t cause, uint64_t value);

/* Handle pending interrupts */
err_t venv_cpu_check_interrupts(venv_cpu_t* cpu, void* memory_context);

/* Read a register (handles zero register) */
static inline uint64_t venv_cpu_read_reg(const venv_cpu_t* cpu, uint8_t reg_idx)
{
    if (reg_idx == VENV_REG_ZERO)
        return 0;
    return cpu->regs[reg_idx];
}

/* Write a register (zero register is read-only) */
static inline err_t venv_cpu_write_reg(venv_cpu_t* cpu, uint8_t reg_idx, uint64_t value)
{
    if (reg_idx == VENV_REG_ZERO)
        return PURE_OK;  /* Silently ignore writes to zero */
    cpu->regs[reg_idx] = value;
    return PURE_OK;
}

/* Set privilege level */
static inline void venv_cpu_set_privilege(venv_cpu_t* cpu, venv_privilege_t priv)
{
    cpu->status.priv = priv;
}

/* Get current privilege level */
static inline venv_privilege_t venv_cpu_get_privilege(const venv_cpu_t* cpu)
{
    return (venv_privilege_t)cpu->status.priv;
}

#endif /* VenVCPU_h */
