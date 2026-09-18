/*
 * VenV - Virtual Environment Engine
 * Test Program
 * 
 * Tests the basic CPU, memory, and assembler functionality.
 */

#include "VenV/VenV.h"
#include "../Std/IO.h"
#include "../Std/Memory.h"

static void print_hex(uint64_t value, int digits)
{
    const char* hex = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--)
    {
        buffer[i] = hex[value & 0xF];
        value >>= 4;
    }
    
    /* Skip leading zeros */
    int start = 16 - digits;
    if (start < 0) start = 0;
    while (start < 15 && buffer[start] == '0') start++;
    
    printf("0x%s", &buffer[start]);
}

static err_t test_cpu_basic(void)
{
    printf("\n=== Test: Basic CPU Operations ===\n");
    
    venv_cpu_t cpu;
    venv_memory_t mem;
    
    /* Initialize memory */
    err_t err = venv_memory_init(&mem, VENV_MEM_DEFAULT_SIZE);
    if (err != PURE_OK)
    {
        printf("FAIL: Memory init failed: %d\n", err);
        return err;
    }
    
    /* Reset CPU */
    err = venv_cpu_reset(&cpu);
    if (err != PURE_OK)
    {
        printf("FAIL: CPU reset failed: %d\n", err);
        venv_memory_destroy(&mem);
        return err;
    }
    
    printf("CPU reset OK\n");
    printf("PC = "); print_hex(cpu.pc, 8); printf("\n");
    printf("SP = "); print_hex(venv_cpu_read_reg(&cpu, VENV_REG_SP), 16); printf("\n");
    
    /* Load a simple program: add x1, x2, x3 */
    /* This is: add x1, x2, x3 -> opcode=OP_OP, func=ADD, rd=1, rs1=2, rs2=3 */
    venv_insn_t insn = 0;
    insn = VENV_ALU_ADD << VENV_FUNC_SHIFT;
    insn |= (3 & VENV_RS2_MASK) << VENV_RS2_SHIFT;
    insn |= (2 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
    insn |= (1 & VENV_RD_MASK) << VENV_RD_SHIFT;
    insn |= VENV_OP_OP << VENV_OPCODE_SHIFT;
    
    /* Write instruction to memory at PC */
    uint8_t insn_bytes[4];
    insn_bytes[0] = insn & 0xFF;
    insn_bytes[1] = (insn >> 8) & 0xFF;
    insn_bytes[2] = (insn >> 16) & 0xFF;
    insn_bytes[3] = (insn >> 24) & 0xFF;
    
    err = venv_memory_write(&mem, cpu.pc, insn_bytes, 4);
    if (err != PURE_OK)
    {
        printf("FAIL: Memory write failed: %d\n", err);
        venv_memory_destroy(&mem);
        return err;
    }
    
    /* Set up registers */
    venv_cpu_write_reg(&cpu, 2, 100);
    venv_cpu_write_reg(&cpu, 3, 50);
    
    printf("Before execution:\n");
    printf("  x2 = %lu\n", venv_cpu_read_reg(&cpu, 2));
    printf("  x3 = %lu\n", venv_cpu_read_reg(&cpu, 3));
    
    /* Execute one instruction */
    err = venv_cpu_step(&cpu, &mem);
    if (err != PURE_OK)
    {
        printf("FAIL: CPU step failed: %d\n", err);
        venv_memory_destroy(&mem);
        return err;
    }
    
    printf("After execution:\n");
    printf("  x1 = %lu (expected: 150)\n", venv_cpu_read_reg(&cpu, 1));
    printf("  PC = "); print_hex(cpu.pc, 8); printf(" (expected: 0x4)\n");
    
    if (venv_cpu_read_reg(&cpu, 1) == 150 && cpu.pc == 4)
    {
        printf("PASS: Basic CPU test\n");
    }
    else
    {
        printf("FAIL: Results don't match expected values\n");
        venv_memory_destroy(&mem);
        return PURE_ERROR_UNKNOWN;
    }
    
    venv_memory_destroy(&mem);
    return PURE_OK;
}

static err_t test_assembler_basic(void)
{
    printf("\n=== Test: Basic Assembler ===\n");
    
    const char* asm_code = 
        "addi x1, zero, 42\n"
        "addi x2, zero, 58\n"
        "add  x3, x1, x2\n"
        "sub  x4, x2, x1\n";
    
    uint8_t* code = NULL;
    uint64_t code_size = 0;
    
    err_t err = venv_asm_assemble_text(asm_code, &code, &code_size);
    if (err != PURE_OK)
    {
        printf("FAIL: Assembly failed: %s\n", venv_asm_get_error());
        return err;
    }
    
    printf("Assembled %lu bytes of code\n", code_size);
    printf("Code: ");
    for (uint64_t i = 0; i < code_size && i < 32; i++)
    {
        printf("%02X ", code[i]);
    }
    printf("\n");
    
    /* Now execute it */
    venv_cpu_t cpu;
    venv_memory_t mem;
    
    err = venv_memory_init(&mem, VENV_MEM_DEFAULT_SIZE);
    if (err != PURE_OK)
    {
        heap_deallocate(code);
        return err;
    }
    
    err = venv_cpu_reset(&cpu);
    if (err != PURE_OK)
    {
        venv_memory_destroy(&mem);
        heap_deallocate(code);
        return err;
    }
    
    /* Load code into memory */
    err = venv_memory_write(&mem, cpu.pc, code, code_size);
    if (err != PURE_OK)
    {
        printf("FAIL: Code load failed: %d\n", err);
        venv_memory_destroy(&mem);
        heap_deallocate(code);
        return err;
    }
    
    /* Execute all instructions */
    uint64_t inst_count = code_size / 4;
    for (uint64_t i = 0; i < inst_count; i++)
    {
        err = venv_cpu_step(&cpu, &mem);
        if (err != PURE_OK)
        {
            printf("FAIL: Execution failed at instruction %lu: %d\n", i, err);
            venv_memory_destroy(&mem);
            heap_deallocate(code);
            return err;
        }
    }
    
    printf("Results:\n");
    printf("  x1 = %lu (expected: 42)\n", venv_cpu_read_reg(&cpu, 1));
    printf("  x2 = %lu (expected: 58)\n", venv_cpu_read_reg(&cpu, 2));
    printf("  x3 = %lu (expected: 100)\n", venv_cpu_read_reg(&cpu, 3));
    printf("  x4 = %lu (expected: 16)\n", venv_cpu_read_reg(&cpu, 4));
    
    if (venv_cpu_read_reg(&cpu, 1) == 42 &&
        venv_cpu_read_reg(&cpu, 2) == 58 &&
        venv_cpu_read_reg(&cpu, 3) == 100 &&
        venv_cpu_read_reg(&cpu, 4) == 16)
    {
        printf("PASS: Assembler test\n");
    }
    else
    {
        printf("FAIL: Results don't match expected values\n");
        venv_memory_destroy(&mem);
        heap_deallocate(code);
        return PURE_ERROR_UNKNOWN;
    }
    
    venv_memory_destroy(&mem);
    heap_deallocate(code);
    return PURE_OK;
}

static err_t test_branch(void)
{
    printf("\n=== Test: Branch Instructions ===\n");
    
    const char* asm_code = 
        "addi x1, zero, 5\n"
        "addi x2, zero, 10\n"
        "blt  x1, x2, branch_taken\n"
        "addi x3, zero, 0\n"
        "beq  x1, x2, branch_not_taken\n"
        "addi x4, zero, 1\n"
        "branch_taken:\n"
        "addi x3, zero, 99\n"
        "branch_not_taken:\n"
        "nop\n";
    
    uint8_t* code = NULL;
    uint64_t code_size = 0;
    
    err_t err = venv_asm_assemble_text(asm_code, &code, &code_size);
    if (err != PURE_OK)
    {
        printf("FAIL: Assembly failed: %s\n", venv_asm_get_error());
        return err;
    }
    
    venv_cpu_t cpu;
    venv_memory_t mem;
    
    err = venv_memory_init(&mem, VENV_MEM_DEFAULT_SIZE);
    if (err != PURE_OK)
    {
        heap_deallocate(code);
        return err;
    }
    
    err = venv_cpu_reset(&cpu);
    if (err != PURE_OK)
    {
        venv_memory_destroy(&mem);
        heap_deallocate(code);
        return err;
    }
    
    err = venv_memory_write(&mem, cpu.pc, code, code_size);
    if (err != PURE_OK)
    {
        printf("FAIL: Code load failed: %d\n", err);
        venv_memory_destroy(&mem);
        heap_deallocate(code);
        return err;
    }
    
    /* Run until we hit an unknown instruction (end of program) */
    for (int i = 0; i < 100; i++)
    {
        err = venv_cpu_step(&cpu, &mem);
        if (err != PURE_OK)
            break;
    }
    
    printf("Results:\n");
    printf("  x1 = %lu (expected: 5)\n", venv_cpu_read_reg(&cpu, 1));
    printf("  x2 = %lu (expected: 10)\n", venv_cpu_read_reg(&cpu, 2));
    printf("  x3 = %lu (expected: 99 - branch was taken)\n", venv_cpu_read_reg(&cpu, 3));
    
    if (venv_cpu_read_reg(&cpu, 1) == 5 &&
        venv_cpu_read_reg(&cpu, 2) == 10 &&
        venv_cpu_read_reg(&cpu, 3) == 99)
    {
        printf("PASS: Branch test\n");
    }
    else
    {
        printf("FAIL: Results don't match expected values\n");
        venv_memory_destroy(&mem);
        heap_deallocate(code);
        return PURE_ERROR_UNKNOWN;
    }
    
    venv_memory_destroy(&mem);
    heap_deallocate(code);
    return PURE_OK;
}

int main(void)
{
    printf("===========================================\n");
    printf("  VenV - Virtual Environment Engine\n");
    printf("  Version: %s\n", VENV_VERSION_STRING);
    printf("===========================================\n");
    
    err_t err;
    
    /* Test 1: Basic CPU operations */
    err = test_cpu_basic();
    if (err != PURE_OK)
    {
        printf("CPU basic test FAILED\n");
        return 1;
    }
    
    /* Test 2: Assembler */
    err = test_assembler_basic();
    if (err != PURE_OK)
    {
        printf("Assembler test FAILED\n");
        return 1;
    }
    
    /* Test 3: Branch instructions */
    err = test_branch();
    if (err != PURE_OK)
    {
        printf("Branch test FAILED\n");
        return 1;
    }
    
    printf("\n===========================================\n");
    printf("  All tests PASSED!\n");
    printf("===========================================\n");
    
    return 0;
}
