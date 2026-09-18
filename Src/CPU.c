/*
 * VenV - Virtual Environment Engine
 * CPU Implementation
 */

#include "../Include/VenV/CPU.h"
#include "../Include/VenV/Memory.h"
#include "../Include/VenV/ISA.h"
#include <xkeycheck.h>

/*---- Helper Macros ----*/

#define DECODE_OPCODE(insn)    (((insn) >> VENV_OPCODE_SHIFT) & VENV_OPCODE_MASK)
#define DECODE_RD(insn)        (((insn) >> VENV_RD_SHIFT) & VENV_RD_MASK)
#define DECODE_RS1(insn)       (((insn) >> VENV_RS1_SHIFT) & VENV_RS1_MASK)
#define DECODE_RS2(insn)       (((insn) >> VENV_RS2_SHIFT) & VENV_RS2_MASK)
#define DECODE_FUNC(insn)      (((insn) >> VENV_FUNC_SHIFT) & VENV_FUNC_MASK)
#define DECODE_IMM12(insn)     (((insn) >> VENV_IMM12_SHIFT) & VENV_IMM12_MASK)
#define DECODE_IMM20(insn)     (((insn) >> VENV_IMM20_SHIFT) & VENV_IMM20_MASK)

/* Sign-extend 12-bit immediate */
static inline int64_t sext_imm12(uint32_t imm)
{
    return (int64_t)((int32_t)imm << 20) >> 20;
}

/* Sign-extend 20-bit immediate */
static inline int64_t sext_imm20(uint32_t imm)
{
    return (int64_t)((int32_t)imm << 12) >> 12;
}

/*---- CPU Reset ----*/

err_t venv_cpu_reset(venv_cpu_t* cpu)
{
    if (cpu == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Clear all registers */
    for (int i = 0; i < VENV_REG_COUNT; i++)
        cpu->regs[i] = 0;

    /* Reset PC to entry point (0x00000000) */
    cpu->pc = 0x00000000;

    /* Reset status register */
    cpu->status.ie = 0;
    cpu->status.pie = 0;
    cpu->status.priv = VENV_PRIV_MACHINE;  /* Start in machine mode */
    cpu->status.wp = 0;

    /* Clear exception state */
    cpu->cause = 0;
    cpu->value = 0;
    cpu->epc = 0;

    /* Clear CSRs */
    for (int i = 0; i < 256; i++)
        cpu->csrs[i] = 0;

    /* Reset counters */
    cpu->cycle = 0;
    cpu->instret = 0;

    return PURE_OK;
}

/*---- Exception Handling ----*/

err_t venv_cpu_raise_exception(venv_cpu_t* cpu, uint64_t cause, uint64_t value)
{
    if (cpu == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Save exception context */
    cpu->epc = cpu->pc;
    cpu->cause = cause;
    cpu->value = value;

    /* Save interrupt enable state */
    cpu->status.pie = cpu->status.ie;
    cpu->status.ie = 0;  /* Disable interrupts */

    /* Set trap vector address based on privilege level */
    uint64_t trap_addr = 0x1000 + (cause * 8);  /* Simple trap vector table */

    /* Jump to trap handler */
    cpu->pc = trap_addr;

    return PURE_OK;
}

/*---- Instruction Execution ----*/

static err_t execute_lui(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t rd = DECODE_RD(insn);
    uint64_t imm = (uint64_t)DECODE_IMM20(insn) << 12;

    return venv_cpu_write_reg(cpu, rd, imm);
}

static err_t execute_auipc(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t rd = DECODE_RD(insn);
    int64_t imm = sext_imm20(DECODE_IMM20(insn));
    uint64_t result = cpu->pc + (imm << 12);

    return venv_cpu_write_reg(cpu, rd, result);
}

static err_t execute_jal(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t rd = DECODE_RD(insn);
    int64_t imm = sext_imm20(DECODE_IMM20(insn));
    uint64_t target = cpu->pc + (imm << 1);  /* JAL uses shifted immediate */

    /* Write return address */
    err_t err = venv_cpu_write_reg(cpu, rd, cpu->pc + 4);
    if (err != PURE_OK)
        return err;

    /* Jump */
    cpu->pc = target;

    return PURE_OK;
}

static err_t execute_jalr(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t rd = DECODE_RD(insn);
    uint8_t rs1 = DECODE_RS1(insn);
    int64_t imm = sext_imm12(DECODE_IMM12(insn));

    uint64_t base = venv_cpu_read_reg(cpu, rs1);
    uint64_t target = (base + imm) & ~1ULL;  /* Clear LSB for alignment */

    /* Write return address */
    err_t err = venv_cpu_write_reg(cpu, rd, cpu->pc + 4);
    if (err != PURE_OK)
        return err;

    /* Jump */
    cpu->pc = target;

    return PURE_OK;
}

static err_t execute_branch(venv_cpu_t* cpu, venv_insn_t insn, venv_memory_t* mem)
{
    uint8_t rs1 = DECODE_RS1(insn);
    uint8_t rs2 = DECODE_RS2(insn);
    uint8_t func = DECODE_FUNC(insn);
    int64_t imm = sext_imm12(DECODE_IMM12(insn));

    uint64_t val1 = venv_cpu_read_reg(cpu, rs1);
    uint64_t val2 = venv_cpu_read_reg(cpu, rs2);

    boolean take_branch = false;

    switch (func)
    {
        case VENV_BRANCH_EQ:
            take_branch = ((int64_t)val1 == (int64_t)val2);
            break;
        case VENV_BRANCH_NE:
            take_branch = ((int64_t)val1 != (int64_t)val2);
            break;
        case VENV_BRANCH_LT:
            take_branch = ((int64_t)val1 < (int64_t)val2);
            break;
        case VENV_BRANCH_GE:
            take_branch = ((int64_t)val1 >= (int64_t)val2);
            break;
        case VENV_BRANCH_LTU:
            take_branch = (val1 < val2);
            break;
        case VENV_BRANCH_GEU:
            take_branch = (val1 >= val2);
            break;
        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }

    if (take_branch)
        cpu->pc = cpu->pc + (imm << 1);  /* Branch uses shifted immediate */
    else
        cpu->pc = cpu->pc + 4;

    return PURE_OK;
}

static err_t execute_load(venv_cpu_t* cpu, venv_insn_t insn, venv_memory_t* mem)
{
    uint8_t rd = DECODE_RD(insn);
    uint8_t rs1 = DECODE_RS1(insn);
    uint8_t size_func = DECODE_FUNC(insn);
    int64_t imm = sext_imm12(DECODE_IMM12(insn));

    uint64_t addr = venv_cpu_read_reg(cpu, rs1) + imm;
    uint64_t value = 0;

    /* Determine load size and sign extension */
    err_t err = PURE_OK;
    switch (size_func & 0x7)
    {
        case VENV_LOAD_BYTE:
        {
            uint8_t v8;
            err = venv_memory_read_u8(mem, addr, &v8);
            value = (uint64_t)(int64_t)(int8_t)v8;  /* Sign-extend */
            break;
        }
        case VENV_LOAD_HALF:
        {
            uint16_t v16;
            err = venv_memory_read_u16(mem, addr, &v16);
            value = (uint64_t)(int64_t)(int16_t)v16;  /* Sign-extend */
            break;
        }
        case VENV_LOAD_WORD:
        {
            uint32_t v32;
            err = venv_memory_read_u32(mem, addr, &v32);
            value = (uint64_t)(int64_t)(int32_t)v32;  /* Sign-extend */
            break;
        }
        case VENV_LOAD_DWORD:
        {
            err = venv_memory_read_u64(mem, addr, &value);
            break;
        }
        case VENV_LOAD_UBYTE:
        {
            uint8_t v8;
            err = venv_memory_read_u8(mem, addr, &v8);
            value = v8;  /* Zero-extend */
            break;
        }
        case VENV_LOAD_UHALF:
        {
            uint16_t v16;
            err = venv_memory_read_u16(mem, addr, &v16);
            value = v16;  /* Zero-extend */
            break;
        }
        case VENV_LOAD_UWORD:
        {
            uint32_t v32;
            err = venv_memory_read_u32(mem, addr, &v32);
            value = v32;  /* Zero-extend */
            break;
        }
        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }

    if (err != PURE_OK)
        return venv_cpu_raise_exception(cpu, VENV_EXC_LOAD_FAULT, addr);

    return venv_cpu_write_reg(cpu, rd, value);
}

static err_t execute_store(venv_cpu_t* cpu, venv_insn_t insn, venv_memory_t* mem)
{
    uint8_t rs1 = DECODE_RS1(insn);
    uint8_t rs2 = DECODE_RS2(insn);
    uint8_t size_func = DECODE_FUNC(insn);
    int64_t imm = sext_imm12(DECODE_IMM12(insn));

    uint64_t addr = venv_cpu_read_reg(cpu, rs1) + imm;
    uint64_t value = venv_cpu_read_reg(cpu, rs2);

    err_t err = PURE_OK;
    switch (size_func & 0xF)
    {
        case VENV_STORE_BYTE:
            err = venv_memory_write_u8(mem, addr, (uint8_t)value);
            break;
        case VENV_STORE_HALF:
            err = venv_memory_write_u16(mem, addr, (uint16_t)value);
            break;
        case VENV_STORE_WORD:
            err = venv_memory_write_u32(mem, addr, (uint32_t)value);
            break;
        case VENV_STORE_DWORD:
            err = venv_memory_write_u64(mem, addr, value);
            break;
        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }

    if (err != PURE_OK)
        return venv_cpu_raise_exception(cpu, VENV_EXC_STORE_FAULT, addr);

    return PURE_OK;
}

static err_t execute_opimm(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t rd = DECODE_RD(insn);
    uint8_t rs1 = DECODE_RS1(insn);
    uint8_t func = DECODE_FUNC(insn);
    int64_t imm = sext_imm12(DECODE_IMM12(insn));

    uint64_t val = venv_cpu_read_reg(cpu, rs1);
    uint64_t result = 0;

    switch (func)
    {
        case VENV_ALU_ADD:
            result = val + imm;
            break;
        case VENV_ALU_AND:
            result = val & imm;
            break;
        case VENV_ALU_OR:
            result = val | imm;
            break;
        case VENV_ALU_XOR:
            result = val ^ imm;
            break;
        case VENV_ALU_SLL:
            result = val << (imm & 63);
            break;
        case VENV_ALU_SRL:
            result = val >> (imm & 63);
            break;
        case VENV_ALU_SRA:
            result = (uint64_t)((int64_t)val >> (imm & 63));
            break;
        case VENV_ALU_SLT:
            result = ((int64_t)val < imm) ? 1 : 0;
            break;
        case VENV_ALU_SLTU:
            result = (val < (uint64_t)imm) ? 1 : 0;
            break;
        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }

    return venv_cpu_write_reg(cpu, rd, result);
}

static err_t execute_op(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t rd = DECODE_RD(insn);
    uint8_t rs1 = DECODE_RS1(insn);
    uint8_t rs2 = DECODE_RS2(insn);
    uint8_t func = DECODE_FUNC(insn);

    uint64_t val1 = venv_cpu_read_reg(cpu, rs1);
    uint64_t val2 = venv_cpu_read_reg(cpu, rs2);
    uint64_t result = 0;

    switch (func)
    {
        case VENV_ALU_ADD:
            result = val1 + val2;
            break;
        case VENV_ALU_SUB:
            result = val1 - val2;
            break;
        case VENV_ALU_AND:
            result = val1 & val2;
            break;
        case VENV_ALU_OR:
            result = val1 | val2;
            break;
        case VENV_ALU_XOR:
            result = val1 ^ val2;
            break;
        case VENV_ALU_SLL:
            result = val1 << (val2 & 63);
            break;
        case VENV_ALU_SRL:
            result = val1 >> (val2 & 63);
            break;
        case VENV_ALU_SRA:
            result = (uint64_t)((int64_t)val1 >> (val2 & 63));
            break;
        case VENV_ALU_SLT:
            result = ((int64_t)val1 < (int64_t)val2) ? 1 : 0;
            break;
        case VENV_ALU_SLTU:
            result = (val1 < val2) ? 1 : 0;
            break;
        case VENV_ALU_MUL:
            result = val1 * val2;
            break;
        case VENV_ALU_DIV:
            if (val2 == 0)
                return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
            result = (uint64_t)((int64_t)val1 / (int64_t)val2);
            break;
        case VENV_ALU_REM:
            if (val2 == 0)
                return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
            result = (uint64_t)((int64_t)val1 % (int64_t)val2);
            break;
        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }

    return venv_cpu_write_reg(cpu, rd, result);
}

static err_t execute_misc(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t func = DECODE_FUNC(insn);

    switch (func)
    {
        case VENV_MISC_FENCE:
            /* Memory fence - no-op in simple implementation */
            break;

        case VENV_MISC_MRET:
            /* Return from trap */
            cpu->pc = cpu->epc;
            cpu->status.ie = cpu->status.pie;
            break;

        case VENV_MISC_WFI:
            /* Wait for interrupt - halt execution */
            return PURE_ERROR_OUT_OF_BOUNDS;  /* Signal VM to stop */

        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }

    return PURE_OK;
}

static err_t execute_trap(venv_cpu_t* cpu, venv_insn_t insn)
{
    uint8_t type = DECODE_FUNC(insn);

    switch (type)
    {
        case VENV_TRAP_ECALL:
            /* Environment call (syscall) */
            return venv_cpu_raise_exception(cpu,
                VENV_EXC_ECALL_USER + cpu->status.priv, 0);

        case VENV_TRAP_EBREAK:
            /* Breakpoint */
            return venv_cpu_raise_exception(cpu, VENV_EXC_BREAKPOINT, cpu->pc);

        case VENV_TRAP_INT:
            /* Software interrupt */
            return venv_cpu_raise_exception(cpu, VENV_INTR_SOFTWARE, 0);

        default:
            return venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
    }
}

/*---- Single Instruction Step ----*/

err_t venv_cpu_step(venv_cpu_t* cpu, void* memory_context)
{
    if (cpu == NULL || memory_context == NULL)
        return PURE_ERROR_NULL_POINTER;

    venv_memory_t* mem = (venv_memory_t*)memory_context;

    /* Fetch instruction */
    venv_insn_t insn;
    err_t err = venv_memory_fetch_insn(mem, cpu->pc, &insn);
    if (err != PURE_OK)
        return venv_cpu_raise_exception(cpu, VENV_EXC_INSTR_FAULT, cpu->pc);

    /* Decode and execute */
    uint8_t opcode = DECODE_OPCODE(insn);

    switch (opcode)
    {
        case VENV_OP_LUI:
            err = execute_lui(cpu, insn);
            break;
        case VENV_OP_AUIPC:
            err = execute_auipc(cpu, insn);
            break;
        case VENV_OP_JAL:
            err = execute_jal(cpu, insn);
            break;
        case VENV_OP_JALR:
            err = execute_jalr(cpu, insn);
            break;
        case VENV_OP_BR:
            err = execute_branch(cpu, insn, mem);
            break;
        case VENV_OP_LD:
            err = execute_load(cpu, insn, mem);
            break;
        case VENV_OP_ST:
            err = execute_store(cpu, insn, mem);
            break;
        case VENV_OP_OPIMM:
            err = execute_opimm(cpu, insn);
            break;
        case VENV_OP_OP:
            err = execute_op(cpu, insn);
            break;
        case VENV_OP_MISC:
            err = execute_misc(cpu, insn);
            break;
        case VENV_OP_TRAP:
            err = execute_trap(cpu, insn);
            break;
        default:
            err = venv_cpu_raise_exception(cpu, VENV_EXC_ILLEGAL_INSN, insn);
            break;
    }

    /* Update counters */
    cpu->cycle++;
    cpu->instret++;

    /* Advance PC if not changed by instruction */
    if (err == PURE_OK && opcode != VENV_OP_JAL && opcode != VENV_OP_JALR
        && opcode != VENV_OP_BR && opcode != VENV_OP_MISC)
        cpu->pc += 4;

    return err;
}

/*---- Run Multiple Instructions ----*/

err_t venv_cpu_run(venv_cpu_t* cpu, void* memory_context, uint64_t count)
{
    if (cpu == NULL || memory_context == NULL)
        return PURE_ERROR_NULL_POINTER;

    for (uint64_t i = 0; i < count; i++)
    {
        err_t err = venv_cpu_step(cpu, memory_context);
        if (err != PURE_OK)
            return err;
    }

    return PURE_OK;
}

/*---- Interrupt Check ----*/

err_t venv_cpu_check_interrupts(venv_cpu_t* cpu, void* memory_context)
{
    if (cpu == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Check if interrupts are enabled */
    if (!cpu->status.ie)
        return PURE_OK;

    /* TODO: Check pending interrupts from devices */
    /* This will be implemented when we have an interrupt controller */

    return PURE_OK;
}
