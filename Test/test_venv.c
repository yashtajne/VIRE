/*
 * VIRE - Virtual Isolated Runtime Environment
 * Test Program
 */

#include "../Include/VenV/VenV.h"
#include "../Include/Interface/Core/ASCII.h"
#include "../Include/Interface/Core/Memory.h"
#include "../Pure.h"

extern void console_putchar(char c);

static void print_uint64(uint64_t value)
{
    if (value == 0) { console_putchar('0'); return; }
    char buffer[21]; int i = 20; buffer[i] = '\0';
    while (value > 0) { buffer[--i] = '0' + (value % 10); value /= 10; }
    while (buffer[i] != '\0') console_putchar(buffer[i++]);
}

static void print_hex(uint64_t value, int digits)
{
    const char* hex = "0123456789ABCDEF";
    char buffer[17]; buffer[16] = '\0';
    for (int i = 15; i >= 0; i--) { buffer[i] = hex[value & 0xF]; value >>= 4; }
    int start = 16 - digits; if (start < 0) start = 0;
    while (start < 15 && buffer[start] == '0') start++;
    console_putchar('0'); console_putchar('x');
    for (int i = start; i < 16; i++) console_putchar(buffer[i]);
}

static void print_string(const char* str) { while (*str) console_putchar(*str++); }
static void print_newline(void) { console_putchar('\n'); }

static err_t test_cpu_basic(void)
{
    print_newline(); print_string("=== Test: Basic CPU Operations ==="); print_newline();
    venv_cpu_t cpu; venv_memory_t mem;
    err_t err = venv_memory_init(&mem, VENV_MEM_DEFAULT_SIZE);
    if (err != PURE_OK) { print_string("FAIL: Memory init"); print_newline(); return err; }
    err = venv_cpu_reset(&cpu);
    if (err != PURE_OK) { print_string("FAIL: CPU reset"); print_newline(); venv_memory_destroy(&mem); return err; }
    print_string("CPU reset OK"); print_newline();
    
    venv_insn_t insn = 0;
    insn = VENV_ALU_ADD << VENV_FUNC_SHIFT;
    insn |= (3 & VENV_RS2_MASK) << VENV_RS2_SHIFT;
    insn |= (2 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
    insn |= (1 & VENV_RD_MASK) << VENV_RD_SHIFT;
    insn |= VENV_OP_OP << VENV_OPCODE_SHIFT;
    
    uint8_t insn_bytes[4];
    insn_bytes[0] = insn & 0xFF; insn_bytes[1] = (insn >> 8) & 0xFF;
    insn_bytes[2] = (insn >> 16) & 0xFF; insn_bytes[3] = (insn >> 24) & 0xFF;
    
    err = venv_memory_write(&mem, cpu.pc, insn_bytes, 4);
    if (err != PURE_OK) { print_string("FAIL: Memory write"); print_newline(); venv_memory_destroy(&mem); return err; }
    
    venv_cpu_write_reg(&cpu, 2, 100); venv_cpu_write_reg(&cpu, 3, 50);
    err = venv_cpu_step(&cpu, &mem);
    if (err != PURE_OK) { print_string("FAIL: CPU step"); print_newline(); venv_memory_destroy(&mem); return err; }
    
    if (venv_cpu_read_reg(&cpu, 1) == 150 && cpu.pc == 4)
        { print_string("PASS: Basic CPU test"); print_newline(); venv_memory_destroy(&mem); return PURE_OK; }
    else
        { print_string("FAIL: Results mismatch"); print_newline(); venv_memory_destroy(&mem); return PURE_ERROR_UNKNOWN; }
}

static err_t test_assembler_basic(void)
{
    print_newline(); print_string("=== Test: Basic Assembler ==="); print_newline();
    const char* asm_code = "addi x1, zero, 42\naddi x2, zero, 58\nadd  x3, x1, x2\nsub  x4, x2, x1\n";
    uint8_t* code = NULL; uint64_t code_size = 0;
    err_t err = venv_asm_assemble_text(asm_code, &code, &code_size);
    if (err != PURE_OK) { print_string("FAIL: Assembly failed"); print_newline(); return err; }
    
    venv_cpu_t cpu; venv_memory_t mem;
    err = venv_memory_init(&mem, VENV_MEM_DEFAULT_SIZE);
    if (err != PURE_OK) { heap_deallocate(code); return err; }
    err = venv_cpu_reset(&cpu);
    if (err != PURE_OK) { venv_memory_destroy(&mem); heap_deallocate(code); return err; }
    err = venv_memory_write(&mem, cpu.pc, code, code_size);
    if (err != PURE_OK) { print_string("FAIL: Code load"); print_newline(); venv_memory_destroy(&mem); heap_deallocate(code); return err; }
    
    uint64_t inst_count = code_size / 4;
    for (uint64_t i = 0; i < inst_count; i++)
        if (venv_cpu_step(&cpu, &mem) != PURE_OK) break;
    
    if (venv_cpu_read_reg(&cpu, 1) == 42 && venv_cpu_read_reg(&cpu, 2) == 58 &&
        venv_cpu_read_reg(&cpu, 3) == 100 && venv_cpu_read_reg(&cpu, 4) == 16)
        { print_string("PASS: Assembler test"); print_newline(); venv_memory_destroy(&mem); heap_deallocate(code); return PURE_OK; }
    else
        { print_string("FAIL: Results mismatch"); print_newline(); venv_memory_destroy(&mem); heap_deallocate(code); return PURE_ERROR_UNKNOWN; }
}

int main(void)
{
    print_string("==========================================="); print_newline();
    print_string("  VIRE - Virtual Isolated Runtime Environment"); print_newline();
    print_string("==========================================="); print_newline();
    
    if (test_cpu_basic() != PURE_OK) return 1;
    if (test_assembler_basic() != PURE_OK) return 1;
    
    print_newline(); print_string("All tests PASSED!"); print_newline();
    return 0;
}
