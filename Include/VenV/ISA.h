#ifndef VenVISA_h
#define VenVISA_h

/*
 * VenV - Virtual Environment Engine
 * Custom CPU Instruction Set Architecture
 *
 * This is a completely new architecture designed for virtual environment emulation.
 * Not based on x86, ARM, RISC-V, MIPS, or any existing architecture.
 *
 * Design goals:
 * - Simple and efficient to emulate
 * - Capable of running general-purpose OS (eventually Linux)
 * - Compact instruction encoding
 * - Support for privileged execution, interrupts, exceptions
 */

#include "../Interface/Core/Int.h"
#include "../Interface/Core/Byte.h"

/*---- Register File ----*/

/* Number of general-purpose registers */
#define VENV_REG_COUNT      16

/* Special register indices */
#define VENV_REG_ZERO       0   /* Always reads as zero */
#define VENV_REG_SP         14  /* Stack pointer */
#define VENV_REG_RA         15  /* Return address (link register) */

/*---- Instruction Encoding ----*/

/* Instruction is 32-bit fixed width */
typedef uint32_t venv_insn_t;

/* Opcode field (bits 0-6) */
#define VENV_OPCODE_MASK    0x7F
#define VENV_OPCODE_SHIFT   0

/* Register fields */
#define VENV_RD_MASK        0x1F
#define VENV_RD_SHIFT       7
#define VENV_RS1_MASK       0x1F
#define VENV_RS1_SHIFT      12
#define VENV_RS2_MASK       0x1F
#define VENV_RS2_SHIFT      17

/* Immediate fields */
#define VENV_IMM12_MASK     0xFFF
#define VENV_IMM12_SHIFT    20
#define VENV_IMM20_MASK     0xFFFFF
#define VENV_IMM20_SHIFT    12

/* Function codes for R-type instructions */
#define VENV_FUNC_MASK      0x7F
#define VENV_FUNC_SHIFT     25

/*---- Opcodes ----*/

typedef enum
{
    VENV_OP_LUI     = 0x01,  /* Load upper immediate */
    VENV_OP_AUIPC   = 0x02,  /* Add upper immediate to PC */
    VENV_OP_JAL     = 0x03,  /* Jump and link */
    VENV_OP_JALR    = 0x04,  /* Jump and link register */
    VENV_OP_BR      = 0x05,  /* Branch */
    VENV_OP_LD      = 0x06,  /* Load */
    VENV_OP_ST      = 0x07,  /* Store */
    VENV_OP_OPIMM   = 0x08,  /* Immediate arithmetic/logic */
    VENV_OP_OP      = 0x09,  /* Register arithmetic/logic */
    VENV_OP_MISC    = 0x0A,  /* Miscellaneous (system, fence, etc.) */
    VENV_OP_TRAP    = 0x0B,  /* Trap/exception generation */
} venv_opcode_t;

/*---- ALU Functions (for OP and OPIMM) ----*/

typedef enum
{
    VENV_ALU_ADD    = 0x00,
    VENV_ALU_SUB    = 0x01,
    VENV_ALU_AND    = 0x02,
    VENV_ALU_OR     = 0x03,
    VENV_ALU_XOR    = 0x04,
    VENV_ALU_SLL    = 0x05,
    VENV_ALU_SRL    = 0x06,
    VENV_ALU_SRA    = 0x07,
    VENV_ALU_SLT    = 0x08,  /* Set less than (signed) */
    VENV_ALU_SLTU   = 0x09,  /* Set less than unsigned */
    VENV_ALU_MUL    = 0x0A,  /* Multiply */
    VENV_ALU_DIV    = 0x0B,  /* Divide */
    VENV_ALU_REM    = 0x0C,  /* Remainder */
} venv_alu_func_t;

/*---- Branch Functions ----*/

typedef enum
{
    VENV_BRANCH_EQ  = 0x00,  /* Equal */
    VENV_BRANCH_NE  = 0x01,  /* Not equal */
    VENV_BRANCH_LT  = 0x02,  /* Less than (signed) */
    VENV_BRANCH_GE  = 0x03,  /* Greater or equal (signed) */
    VENV_BRANCH_LTU = 0x04,  /* Less than unsigned */
    VENV_BRANCH_GEU = 0x05,  /* Greater or equal unsigned */
} venv_branch_func_t;

/*---- Load/Store Sizes ----*/

typedef enum
{
    VENV_LOAD_BYTE   = 0x00,
    VENV_LOAD_HALF   = 0x01,
    VENV_LOAD_WORD   = 0x02,
    VENV_LOAD_DWORD  = 0x03,
    VENV_LOAD_UBYTE  = 0x04,  /* Zero-extended byte */
    VENV_LOAD_UHALF  = 0x05,  /* Zero-extended half */
    VENV_LOAD_UWORD  = 0x06,  /* Zero-extended word */

    VENV_STORE_BYTE  = 0x08,
    VENV_STORE_HALF  = 0x09,
    VENV_STORE_WORD  = 0x0A,
    VENV_STORE_DWORD = 0x0B,
} venv_mem_size_t;

/*---- System/Misc Functions ----*/

typedef enum
{
    VENV_MISC_FENCE   = 0x00,
    VENV_MISC_MRET    = 0x01,  /* Return from trap/interrupt */
    VENV_MISC_WFI     = 0x02,  /* Wait for interrupt */
    VENV_MISC_CSR_RW  = 0x03,  /* CSR read/write */
    VENV_MISC_CSR_RS  = 0x04,  /* CSR read and set bits */
    VENV_MISC_CSR_RC  = 0x05,  /* CSR read and clear bits */
} venv_misc_func_t;

/*---- Trap Types ----*/

typedef enum
{
    VENV_TRAP_ECALL   = 0x00,  /* Environment call (syscall) */
    VENV_TRAP_EBREAK  = 0x01,  /* Environment break (debugger) */
    VENV_TRAP_INT     = 0x02,  /* Software interrupt */
} venv_trap_type_t;

/*---- Privilege Levels ----*/

typedef enum
{
    VENV_PRIV_USER    = 0,    /* User mode */
    VENV_PRIV_SUPER   = 1,    /* Supervisor mode */
    VENV_PRIV_MACHINE = 2,    /* Machine mode (most privileged) */
} venv_privilege_t;

/*---- Exception/Interrupt Codes ----*/

typedef enum
{
    VENV_EXC_INSTR_ALIGN    = 0,   /* Instruction address misaligned */
    VENV_EXC_INSTR_FAULT    = 1,   /* Instruction access fault */
    VENV_EXC_ILLEGAL_INSN   = 2,   /* Illegal instruction */
    VENV_EXC_BREAKPOINT     = 3,   /* Breakpoint */
    VENV_EXC_LOAD_ALIGN     = 4,   /* Load address misaligned */
    VENV_EXC_LOAD_FAULT     = 5,   /* Load access fault */
    VENV_EXC_STORE_ALIGN    = 6,   /* Store address misaligned */
    VENV_EXC_STORE_FAULT    = 7,   /* Store access fault */
    VENV_EXC_ECALL_USER     = 8,   /* Environment call from U-mode */
    VENV_EXC_ECALL_SUPER    = 9,   /* Environment call from S-mode */
    VENV_EXC_ECALL_MACHINE  = 10,  /* Environment call from M-mode */
    VENV_EXC_INSTR_PAGE     = 11,  /* Instruction page fault */
    VENV_EXC_LOAD_PAGE      = 12,  /* Load page fault */
    VENV_EXC_STORE_PAGE     = 13,  /* Store page fault */

    /* Interrupts (have bit 63 set in some conventions) */
    VENV_INTR_TIMER         = 16,  /* Timer interrupt */
    VENV_INTR_EXTERNAL      = 17,  /* External interrupt */
    VENV_INTR_SOFTWARE      = 18,  /* Software interrupt */
} venv_exception_code_t;

#endif /* VenVISA_h */
