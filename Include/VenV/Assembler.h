#ifndef VenVAssembler_h
#define VenVAssembler_h

/*
 * VenV - Virtual Environment Engine
 * Assembler for VenV ISA
 * 
 * Converts assembly text to machine code.
 * Simple one-pass assembler for initial implementation.
 */

#include "../Core/Int.h"
#include "../Core/Error.h"
#include "ISA.h"

/*---- Assembly Source ----*/

struct VenVAsmLine
{
    uint64_t line_num;          /* Line number in source */
    char* text;                 /* Original line text */
    char* label;                /* Label (if any) */
    char* opcode;               /* Opcode mnemonic */
    char* operands[4];          /* Operands */
    int operand_count;
    venv_insn_t insn;           /* Encoded instruction */
    boolean is_directive;       /* Is a directive (.word, .org, etc.) */
    uint64_t directive_value;   /* Value for directives */
};

typedef struct VenVAsmLine venv_asm_line_t;

struct VenVAsmSource
{
    venv_asm_line_t* lines;
    uint64_t line_count;
    uint64_t capacity;
    
    /* Symbol table */
    char** symbols;
    uint64_t* symbol_values;
    uint64_t symbol_count;
    uint64_t symbol_capacity;
    
    /* Current address (for .org) */
    uint64_t current_addr;
};

typedef struct VenVAsmSource venv_asm_source_t;

/*---- Assembler Operations ----*/

/* Initialize assembler source */
err_t venv_asm_init(venv_asm_source_t* src);

/* Free assembler source */
void venv_asm_destroy(venv_asm_source_t* src);

/* Parse assembly text into internal representation */
err_t venv_asm_parse(venv_asm_source_t* src, const char* asm_text);

/* Load assembly from file */
err_t venv_asm_load_file(venv_asm_source_t* src, const char* filename);

/* Assemble parsed source into machine code */
err_t venv_asm_assemble(venv_asm_source_t* src, uint8_t** out_code, uint64_t* out_size);

/* Assemble directly from text to binary */
err_t venv_asm_assemble_text(const char* asm_text, uint8_t** out_code, uint64_t* out_size);

/* Assemble directly from file to binary */
err_t venv_asm_assemble_file(const char* filename, uint8_t** out_code, uint64_t* out_size);

/* Get error message from last operation */
const char* venv_asm_get_error(void);

/*---- Register Names ----*/

/* Special register aliases */
#define VENV_ASM_REG_ZERO   "zero"  /* x0 - always zero */
#define VENV_ASM_REG_RA     "ra"    /* x15 - return address */
#define VENV_ASM_REG_SP     "sp"    /* x14 - stack pointer */

#endif /* VenVAssembler_h */
