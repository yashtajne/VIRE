/*
 * VIRE - Virtual Isolated Runtime Environment
 * Assembler Implementation
 */

#include "../Include/VenV/ISA.h"
#include "../Include/VenV/Assembler.h"
#include "../Include/Interface/Std/Memory.h"
#include "../Include/Interface/Core/String.h"
#include "../Include/Interface/Core/ASCII.h"
#include "../Include/Pure.h"

/*---- Global Error Message ----*/

static char g_asm_error[256] = {0};

static void set_error(const char* fmt)
{
    /* Simple error setting - copy string manually */
    uint64_t i = 0;
    while (i < sizeof(g_asm_error) - 1 && fmt[i] != '\0')
    {
        g_asm_error[i] = fmt[i];
        i++;
    }
    g_asm_error[i] = '\0';
}

const char* venv_asm_get_error(void)
{
    return g_asm_error;
}

/*---- Helper Functions ----*/

static char* trim_whitespace(char* str)
{
    if (str == NULL)
        return NULL;

    /* Trim leading whitespace */
    while (string_is_space(*str))
        str++;

    if (*str == '\0')
        return str;

    /* Trim trailing whitespace */
    uint64_t len = string_length(str);
    char* end = str + len - 1;
    while (end > str && string_is_space(*end))
        end--;

    *(end + 1) = '\0';
    return str;
}

static int find_register(const char* name)
{
    if (name == NULL)
        return -1;

    /* Check for numeric register (x0-x15) */
    if (name[0] == 'x' || name[0] == 'r')
    {
        int64_t reg = 0;
        err_t err = ascii_to_integer((charseq_t)(name + 1), &reg);
        if (err == PURE_OK && reg >= 0 && reg < VENV_REG_COUNT)
            return (int)reg;
    }

    /* Check aliases */
    if (string_compare(name, "zero") == 0) return VENV_REG_ZERO;
    if (string_compare(name, "ra") == 0) return VENV_REG_RA;
    if (string_compare(name, "sp") == 0) return VENV_REG_SP;

    return -1;
}

static uint64_t parse_immediate(const char* str, boolean* ok)
{
    *ok = false;

    if (str == NULL || *str == '\0')
        return 0;

    /* Handle hex */
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        /* Parse hex manually */
        const char* p = str + 2;
        uint64_t val = 0;
        while (*p != '\0')
        {
            val <<= 4;
            if (*p >= '0' && *p <= '9')
                val |= (*p - '0');
            else if (*p >= 'a' && *p <= 'f')
                val |= (*p - 'a' + 10);
            else if (*p >= 'A' && *p <= 'F')
                val |= (*p - 'A' + 10);
            else
                return 0;  /* Invalid hex digit */
            p++;
        }
        *ok = true;
        return val;
    }

    /* Handle decimal */
    char* end;
    int64_t val = strtoll(str, &end, 10);
    if (*end == '\0')
    {
        *ok = true;
        return (uint64_t)val;
    }

    return 0;
}

/*---- Assembler Initialization ----*/

err_t venv_asm_init(venv_asm_source_t* src)
{
    if (src == NULL)
        return PURE_ERROR_NULL_POINTER;

    src->lines = NULL;
    src->line_count = 0;
    src->capacity = 0;
    src->symbols = NULL;
    src->symbol_values = NULL;
    src->symbol_count = 0;
    src->symbol_capacity = 0;
    src->current_addr = 0;

    return PURE_OK;
}

void venv_asm_destroy(venv_asm_source_t* src)
{
    if (src == NULL)
        return;

    /* Free lines */
    if (src->lines != NULL)
    {
        for (uint64_t i = 0; i < src->line_count; i++)
        {
            venv_asm_line_t* line = &src->lines[i];
            if (line->text != NULL)
                heap_deallocate(line->text);
            if (line->label != NULL)
                heap_deallocate(line->label);
            if (line->opcode != NULL)
                heap_deallocate(line->opcode);

            for (int j = 0; j < line->operand_count; j++)
                if (line->operands[j] != NULL)
                    heap_deallocate(line->operands[j]);
        }
        heap_deallocate(src->lines);
    }

    /* Free symbols */
    if (src->symbols != NULL)
    {
        for (uint64_t i = 0; i < src->symbol_count; i++)
            if (src->symbols[i] != NULL)
                heap_deallocate(src->symbols[i]);
        heap_deallocate(src->symbols);
        heap_deallocate(src->symbol_values);
    }
}

/*---- Parsing ----*/

static err_t add_symbol(venv_asm_source_t* src, const char* name, uint64_t value)
{
    if (src->symbol_count >= src->symbol_capacity)
    {
        uint64_t new_cap = src->symbol_capacity == 0 ? 32 : src->symbol_capacity * 2;
        char** new_symbols = NULL;
        uint64_t* new_values = NULL;

        err_t err = heap_allocate(new_cap * sizeof(char*), (voidptr_t*)&new_symbols);
        if (err != PURE_OK) return err;

        err = heap_allocate(new_cap * sizeof(uint64_t), (voidptr_t*)&new_values);
        if (err != PURE_OK)
        {
            heap_deallocate(new_symbols);
            return err;
        }

        if (src->symbols != NULL)
        {
            for (uint64_t i = 0; i < src->symbol_count; i++)
            {
                new_symbols[i] = src->symbols[i];
                new_values[i] = src->symbol_values[i];
            }
            heap_deallocate(src->symbols);
            heap_deallocate(src->symbol_values);
        }

        src->symbols = new_symbols;
        src->symbol_values = new_values;
        src->symbol_capacity = new_cap;
    }

    /* Copy symbol name */
    uint64_t name_len = string_length(name);
    err_t err = heap_allocate(name_len + 1, (voidptr_t*)&src->symbols[src->symbol_count]);
    if (err != PURE_OK)
        return err;

    string_copy(src->symbols[src->symbol_count], name);
    src->symbol_values[src->symbol_count] = value;
    src->symbol_count++;

    return PURE_OK;
}

static uint64_t find_symbol(venv_asm_source_t* src, const char* name)
{
    for (uint64_t i = 0; i < src->symbol_count; i++)
    {
        if (string_compare(src->symbols[i], name) == 0)
            return src->symbol_values[i];
    }
    return 0xFFFFFFFFFFFFFFFF;  /* Not found */
}

err_t venv_asm_parse(venv_asm_source_t* src, const char* asm_text)
{
    if (src == NULL || asm_text == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Make a copy of the text to tokenize */
    uint64_t text_len = string_length(asm_text);
    char* text_copy = NULL;
    err_t err = heap_allocate(text_len + 1, (voidptr_t*)&text_copy);
    if (err != PURE_OK)
        return err;

    string_copy(text_copy, asm_text);

    /* Parse line by line */
    char* line_start = text_copy;
    uint64_t line_num = 1;

    while (line_start != NULL && *line_start != '\0')
    {
        /* Find end of line */
        char* line_end = NULL;
        uint64_t idx = 0;
        while (line_start[idx] != '\0')
        {
            if (line_start[idx] == '\n')
            {
                line_end = &line_start[idx];
                break;
            }
            idx++;
        }
        if (line_end != NULL)
            *line_end = '\0';

        /* Trim whitespace */
        char* trimmed = trim_whitespace(line_start);

        /* Skip empty lines and comments */
        if (*trimmed != '\0' && *trimmed != '#' && *trimmed != ';')
        {
            /* Grow lines array */
            if (src->line_count >= src->capacity)
            {
                uint64_t new_cap = src->capacity == 0 ? 64 : src->capacity * 2;
                venv_asm_line_t* new_lines = NULL;
                err = heap_allocate(new_cap * sizeof(venv_asm_line_t), (voidptr_t*)&new_lines);
                if (err != PURE_OK)
                {
                    heap_deallocate(text_copy);
                    return err;
                }

                if (src->lines != NULL)
                {
                    memory_copy(src->lines, new_lines, src->line_count * sizeof(venv_asm_line_t));
                    heap_deallocate(src->lines);
                }

                src->lines = new_lines;
                src->capacity = new_cap;
            }

            venv_asm_line_t* line = &src->lines[src->line_count];
            /* Manual memset replacement */
            for (uint64_t i = 0; i < sizeof(venv_asm_line_t); i++)
                ((char*)line)[i] = 0;
            line->line_num = line_num;

            /* Store original text */
            uint64_t len = string_length(trimmed);
            err = heap_allocate(len + 1, (voidptr_t*)&line->text);
            if (err != PURE_OK)
            {
                heap_deallocate(text_copy);
                return err;
            }
            string_copy(line->text, trimmed);

            /* Check for label - manual strchr replacement */
            char* colon = NULL;
            uint64_t idx2 = 0;
            while (trimmed[idx2] != '\0')
            {
                if (trimmed[idx2] == ':')
                {
                    colon = &trimmed[idx2];
                    break;
                }
                idx2++;
            }
            if (colon != NULL)
            {
                *colon = '\0';
                char* label_name = trim_whitespace(trimmed);

                uint64_t label_len = string_length(label_name);
                err = heap_allocate(label_len + 1, (voidptr_t*)&line->label);
                if (err != PURE_OK)
                {
                    heap_deallocate(text_copy);
                    return err;
                }
                string_copy(line->label, label_name);

                /* Add symbol */
                add_symbol(src, label_name, src->current_addr);

                trimmed = trim_whitespace(colon + 1);
            }

            /* Check for directive */
            if (*trimmed == '.')
            {
                line->is_directive = true;
                /* Handle directives like .org, .word, etc. */
                /* Simplified for now */
            }
            else
            {
                /* Parse opcode and operands */
                char* token = trimmed;

                /* Manual strchr replacements */
                char* space = NULL;
                char* tab = NULL;
                char* comma = NULL;
                uint64_t idx3 = 0;
                while (token[idx3] != '\0')
                {
                    if (token[idx3] == ' ' && space == NULL) space = &token[idx3];
                    else if (token[idx3] == '\t' && tab == NULL) tab = &token[idx3];
                    else if (token[idx3] == ',' && comma == NULL) comma = &token[idx3];
                    idx3++;
                }

                /* Find end of opcode */
                char* op_end = NULL;
                if (space != NULL && (op_end == NULL || space < op_end)) op_end = space;
                if (tab != NULL && (op_end == NULL || tab < op_end)) op_end = tab;

                if (op_end != NULL)
                {
                    *op_end = '\0';

                    uint64_t op_len = op_end - token;
                    err = heap_allocate(op_len + 1, (voidptr_t*)&line->opcode);
                    if (err != PURE_OK)
                    {
                        heap_deallocate(text_copy);
                        return err;
                    }
                    string_copy_n(line->opcode, token, op_len);

                    /* Parse operands */
                    char* rest = op_end + 1;
                    while (*rest != '\0' && line->operand_count < 4)
                    {
                        rest = trim_whitespace(rest);
                        if (*rest == '\0') break;

                        /* Manual strchr for comma */
                        char* next_comma = NULL;
                        uint64_t idx4 = 0;
                        while (rest[idx4] != '\0')
                        {
                            if (rest[idx4] == ',')
                            {
                                next_comma = &rest[idx4];
                                break;
                            }
                            idx4++;
                        }
                        char* operand_end = next_comma != NULL ? next_comma : rest + string_length(rest);

                        if (next_comma != NULL)
                            *next_comma = '\0';

                        uint64_t op_len2 = string_length(rest);
                        if (op_len2 > 0)
                        {
                            err = heap_allocate(op_len2 + 1, (voidptr_t*)&line->operands[line->operand_count]);
                            if (err != PURE_OK)
                            {
                                heap_deallocate(text_copy);
                                return err;
                            }
                            string_copy(line->operands[line->operand_count], rest);
                            line->operand_count++;
                        }

                        rest = next_comma != NULL ? next_comma + 1 : operand_end;
                    }
                }
                else
                {
                    /* Just opcode, no operands */
                    uint64_t op_len = string_length(trimmed);
                    err = heap_allocate(op_len + 1, (voidptr_t*)&line->opcode);
                    if (err != PURE_OK)
                    {
                        heap_deallocate(text_copy);
                        return err;
                    }
                    string_copy(line->opcode, trimmed);
                }
            }

            /* Advance address */
            if (!line->is_directive)
                src->current_addr += 4;  /* Each instruction is 4 bytes */

            src->line_count++;
        }

        /* Move to next line */
        if (line_end != NULL)
        {
            line_start = line_end + 1;
            line_num++;
        }
        else
        {
            break;
        }
    }

    heap_deallocate(text_copy);
    return PURE_OK;
}

err_t venv_asm_load_file(venv_asm_source_t* src, const char* filename)
{
    if (src == NULL || filename == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* TODO: Implement file loading */
    set_error("File loading not yet implemented");
    return PURE_ERROR_FILE_READ_FAILED;
}

/*---- Assembly Generation ----*/

static err_t encode_instruction(venv_asm_line_t* line, venv_insn_t* out_insn)
{
    if (line->opcode == NULL)
        return PURE_ERROR_INVALID_ARGUMENT;

    venv_insn_t insn = 0;

    /* Map mnemonic to opcode */
    struct { const char* mnemonic; uint8_t opcode; uint8_t func; } opcodes[] = {
        {"lui",     VENV_OP_LUI,     0},
        {"auipc",   VENV_OP_AUIPC,   0},
        {"jal",     VENV_OP_JAL,     0},
        {"jalr",    VENV_OP_JALR,    0},
        {"beq",     VENV_OP_BR,      VENV_BRANCH_EQ},
        {"bne",     VENV_OP_BR,      VENV_BRANCH_NE},
        {"blt",     VENV_OP_BR,      VENV_BRANCH_LT},
        {"bge",     VENV_OP_BR,      VENV_BRANCH_GE},
        {"bltu",    VENV_OP_BR,      VENV_BRANCH_LTU},
        {"bgeu",    VENV_OP_BR,      VENV_BRANCH_GEU},
        {"lb",      VENV_OP_LD,      VENV_LOAD_BYTE},
        {"lh",      VENV_OP_LD,      VENV_LOAD_HALF},
        {"lw",      VENV_OP_LD,      VENV_LOAD_WORD},
        {"ld",      VENV_OP_LD,      VENV_LOAD_DWORD},
        {"lbu",     VENV_OP_LD,      VENV_LOAD_UBYTE},
        {"lhu",     VENV_OP_LD,      VENV_LOAD_UHALF},
        {"lwu",     VENV_OP_LD,      VENV_LOAD_UWORD},
        {"sb",      VENV_OP_ST,      VENV_STORE_BYTE},
        {"sh",      VENV_OP_ST,      VENV_STORE_HALF},
        {"sw",      VENV_OP_ST,      VENV_STORE_WORD},
        {"sd",      VENV_OP_ST,      VENV_STORE_DWORD},
        {"addi",    VENV_OP_OPIMM,   VENV_ALU_ADD},
        {"andi",    VENV_OP_OPIMM,   VENV_ALU_AND},
        {"ori",     VENV_OP_OPIMM,   VENV_ALU_OR},
        {"xori",    VENV_OP_OPIMM,   VENV_ALU_XOR},
        {"slli",    VENV_OP_OPIMM,   VENV_ALU_SLL},
        {"srli",    VENV_OP_OPIMM,   VENV_ALU_SRL},
        {"srai",    VENV_OP_OPIMM,   VENV_ALU_SRA},
        {"slti",    VENV_OP_OPIMM,   VENV_ALU_SLT},
        {"sltiu",   VENV_OP_OPIMM,   VENV_ALU_SLTU},
        {"add",     VENV_OP_OP,      VENV_ALU_ADD},
        {"sub",     VENV_OP_OP,      VENV_ALU_SUB},
        {"and",     VENV_OP_OP,      VENV_ALU_AND},
        {"or",      VENV_OP_OP,      VENV_ALU_OR},
        {"xor",     VENV_OP_OP,      VENV_ALU_XOR},
        {"sll",     VENV_OP_OP,      VENV_ALU_SLL},
        {"srl",     VENV_OP_OP,      VENV_ALU_SRL},
        {"sra",     VENV_OP_OP,      VENV_ALU_SRA},
        {"slt",     VENV_OP_OP,      VENV_ALU_SLT},
        {"sltu",    VENV_OP_OP,      VENV_ALU_SLTU},
        {"mul",     VENV_OP_OP,      VENV_ALU_MUL},
        {"div",     VENV_OP_OP,      VENV_ALU_DIV},
        {"rem",     VENV_OP_OP,      VENV_ALU_REM},
        {"fence",   VENV_OP_MISC,    VENV_MISC_FENCE},
        {"mret",    VENV_OP_MISC,    VENV_MISC_MRET},
        {"wfi",     VENV_OP_MISC,    VENV_MISC_WFI},
        {"ecall",   VENV_OP_TRAP,    VENV_TRAP_ECALL},
        {"ebreak",  VENV_OP_TRAP,    VENV_TRAP_EBREAK},
        {"swi",     VENV_OP_TRAP,    VENV_TRAP_INT},
    };

    int num_opcodes = sizeof(opcodes) / sizeof(opcodes[0]);
    int found = -1;

    for (int i = 0; i < num_opcodes; i++)
    {
        if (string_compare(line->opcode, opcodes[i].mnemonic) == 0)
        {
            found = i;
            break;
        }
    }

    if (found < 0)
    {
        set_error("Unknown opcode");
        return PURE_ERROR_INVALID_ARGUMENT;
    }

    uint8_t opcode = opcodes[found].opcode;
    uint8_t func = opcodes[found].func;

    /* Encode based on instruction type */
    switch (opcode)
    {
        case VENV_OP_LUI:
        {
            if (line->operand_count < 2)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rd = find_register(line->operands[0]);
            boolean ok;
            uint64_t imm = parse_immediate(line->operands[1], &ok);
            if (rd < 0 || !ok)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = (imm >> 12) << VENV_IMM20_SHIFT;
            insn |= (rd & VENV_RD_MASK) << VENV_RD_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_AUIPC:
        case VENV_OP_JAL:
        {
            if (line->operand_count < 2)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rd = find_register(line->operands[0]);

            /* Second operand can be register or immediate/label */
            int rs = find_register(line->operands[1]);
            boolean ok;
            uint64_t imm = parse_immediate(line->operands[1], &ok);

            if (rs >= 0)
                imm = rs;  /* Use register value as offset (unusual but allowed) */

            if (rd < 0)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = ((imm >> 12) & VENV_IMM20_MASK) << VENV_IMM20_SHIFT;
            insn |= (rd & VENV_RD_MASK) << VENV_RD_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_JALR:
        {
            if (line->operand_count < 2)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rd = find_register(line->operands[0]);
            int rs1 = find_register(line->operands[1]);
            int64_t imm = 0;

            if (line->operand_count >= 3)
            {
                boolean ok;
                imm = (int64_t)parse_immediate(line->operands[2], &ok);
            }

            if (rd < 0 || rs1 < 0)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = (imm & VENV_IMM12_MASK) << VENV_IMM12_SHIFT;
            insn |= (rs1 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
            insn |= (rd & VENV_RD_MASK) << VENV_RD_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_BR:
        {
            if (line->operand_count < 3)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rs1 = find_register(line->operands[0]);
            int rs2 = find_register(line->operands[1]);

            /* Third operand is target (label or immediate) */
            boolean ok;
            uint64_t target = parse_immediate(line->operands[2], &ok);
            if (!ok)
            {
                /* It's a label - will need fixup */
                target = 0;  /* Placeholder */
            }

            if (rs1 < 0 || rs2 < 0)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = func << VENV_FUNC_SHIFT;
            insn |= (rs2 & VENV_RS2_MASK) << VENV_RS2_SHIFT;
            insn |= (rs1 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
            insn |= ((target >> 1) & VENV_IMM12_MASK) << VENV_IMM12_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_LD:
        case VENV_OP_ST:
        {
            if (line->operand_count < 2)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rd_or_rs2;
            int rs1;
            int64_t imm = 0;

            if (opcode == VENV_OP_LD)
            {
                /* ld rd, offset(rs1) */
                rd_or_rs2 = find_register(line->operands[0]);
                
                /* Parse offset(rs1) format - manual strchr replacement */
                char* paren = NULL;
                uint64_t idx_p = 0;
                while (line->operands[1][idx_p] != '\0')
                {
                    if (line->operands[1][idx_p] == '(')
                    {
                        paren = &line->operands[1][idx_p];
                        break;
                    }
                    idx_p++;
                }
                if (paren != NULL)
                {
                    *paren = '\0';
                    boolean ok;
                    imm = (int64_t)parse_immediate(line->operands[1], &ok);
                    if (!ok) imm = 0;

                    rs1 = find_register(paren + 1);
                }
                else
                {
                    rs1 = find_register(line->operands[1]);
                }
            }
            else
            {
                /* sd rs2, offset(rs1) */
                rd_or_rs2 = find_register(line->operands[0]);
                
                /* Parse offset(rs1) format - manual strchr replacement */
                char* paren = NULL;
                uint64_t idx_p = 0;
                while (line->operands[1][idx_p] != '\0')
                {
                    if (line->operands[1][idx_p] == '(')
                    {
                        paren = &line->operands[1][idx_p];
                        break;
                    }
                    idx_p++;
                }
                if (paren != NULL)
                {
                    *paren = '\0';
                    boolean ok;
                    imm = (int64_t)parse_immediate(line->operands[1], &ok);
                    if (!ok) imm = 0;

                    rs1 = find_register(paren + 1);
                }
                else
                {
                    rs1 = find_register(line->operands[1]);
                }
            }

            if (rd_or_rs2 < 0 || rs1 < 0)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = func << VENV_FUNC_SHIFT;
            insn |= (rd_or_rs2 & VENV_RS2_MASK) << VENV_RS2_SHIFT;
            insn |= (rs1 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
            insn |= (imm & VENV_IMM12_MASK) << VENV_IMM12_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_OPIMM:
        {
            if (line->operand_count < 3)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rd = find_register(line->operands[0]);
            int rs1 = find_register(line->operands[1]);
            boolean ok;
            uint64_t imm = parse_immediate(line->operands[2], &ok);
            if (!ok) imm = 0;

            if (rd < 0 || rs1 < 0)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = func << VENV_FUNC_SHIFT;
            insn |= (imm & VENV_IMM12_MASK) << VENV_IMM12_SHIFT;
            insn |= (rs1 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
            insn |= (rd & VENV_RD_MASK) << VENV_RD_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_OP:
        {
            if (line->operand_count < 3)
                return PURE_ERROR_INVALID_ARGUMENT;

            int rd = find_register(line->operands[0]);
            int rs1 = find_register(line->operands[1]);
            int rs2 = find_register(line->operands[2]);

            if (rd < 0 || rs1 < 0 || rs2 < 0)
                return PURE_ERROR_INVALID_ARGUMENT;

            insn = func << VENV_FUNC_SHIFT;
            insn |= (rs2 & VENV_RS2_MASK) << VENV_RS2_SHIFT;
            insn |= (rs1 & VENV_RS1_MASK) << VENV_RS1_SHIFT;
            insn |= (rd & VENV_RD_MASK) << VENV_RD_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        case VENV_OP_MISC:
        case VENV_OP_TRAP:
        {
            insn = func << VENV_FUNC_SHIFT;
            insn |= opcode << VENV_OPCODE_SHIFT;
            break;
        }

        default:
            return PURE_ERROR_INVALID_ARGUMENT;
    }

    *out_insn = insn;
    return PURE_OK;
}

err_t venv_asm_assemble(venv_asm_source_t* src, uint8_t** out_code, uint64_t* out_size)
{
    if (src == NULL || out_code == NULL || out_size == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Calculate total size */
    uint64_t code_size = src->line_count * 4;

    /* Allocate output buffer */
    err_t err = heap_allocate(code_size, (voidptr_t*)out_code);
    if (err != PURE_OK)
        return err;
    
    /* Initialize to zero - manual memset replacement */
    uint64_t init_idx = 0;
    while (init_idx < code_size)
    {
        (*out_code)[init_idx] = 0;
        init_idx++;
    }
    
    /* Assemble each line */
    for (uint64_t i = 0; i < src->line_count; i++)
    {
        venv_asm_line_t* line = &src->lines[i];

        if (line->opcode != NULL && !line->is_directive)
        {
            venv_insn_t insn;
            err = encode_instruction(line, &insn);
            if (err != PURE_OK)
            {
                heap_deallocate(*out_code);
                *out_code = NULL;
                return err;
            }

            /* Write instruction (little-endian) */
            uint8_t* ptr = *out_code + (i * 4);
            ptr[0] = insn & 0xFF;
            ptr[1] = (insn >> 8) & 0xFF;
            ptr[2] = (insn >> 16) & 0xFF;
            ptr[3] = (insn >> 24) & 0xFF;
        }
    }

    *out_size = code_size;
    return PURE_OK;
}

err_t venv_asm_assemble_text(const char* asm_text, uint8_t** out_code, uint64_t* out_size)
{
    if (asm_text == NULL || out_code == NULL || out_size == NULL)
        return PURE_ERROR_NULL_POINTER;

    venv_asm_source_t src;
    err_t err = venv_asm_init(&src);
    if (err != PURE_OK)
        return err;

    err = venv_asm_parse(&src, asm_text);
    if (err != PURE_OK)
    {
        venv_asm_destroy(&src);
        return err;
    }

    err = venv_asm_assemble(&src, out_code, out_size);

    venv_asm_destroy(&src);
    return err;
}

err_t venv_asm_assemble_file(const char* filename, uint8_t** out_code, uint64_t* out_size)
{
    if (filename == NULL || out_code == NULL || out_size == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* TODO: Implement file assembly */
    set_error("File assembly not yet implemented");
    return PURE_ERROR_FILE_READ_FAILED;
}
