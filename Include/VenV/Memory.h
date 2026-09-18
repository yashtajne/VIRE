#ifndef VenVMemory_h
#define VenVMemory_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Memory Subsystem
 *
 * Design goals:
 * - Sparse/lazy allocation to minimize host memory usage
 * - Efficient access for the hot execution path
 * - Support for memory protection (eventually needed by Linux)
 * - Extensible for future MMU features
 */

#include "../Interface/Core/Int.h"
#include "../Interface/Core/Error.h"
#include "ISA.h"

/*---- Memory Configuration ----*/

#define VENV_MEM_PAGE_SIZE      4096        /* Page size (matches Linux typical page size) */
#define VENV_MEM_PAGE_SHIFT     12          /* log2(PAGE_SIZE) */
#define VENV_MEM_DEFAULT_SIZE   (256ULL * 1024 * 1024)  /* Default 256MB RAM */

/*---- Memory Protection Flags ----*/

typedef enum
{
    VENV_MEM_READ     = 0x1,
    VENV_MEM_WRITE    = 0x2,
    VENV_MEM_EXECUTE  = 0x4,
} venv_mem_flags_t;

/*---- Memory Region ----*/

struct VenVMemRegion
{
    uint64_t base;              /* Base address */
    uint64_t size;              /* Size in bytes */
    uint8_t* data;              /* Pointer to actual memory (NULL if not allocated) */
    venv_mem_flags_t flags;     /* Access flags */
    boolean is_mapped;          /* Whether region is mapped */
};

typedef struct VenVMemRegion venv_mem_region_t;

/*---- Virtual Memory Context ----*/

struct VenVMemoryContext
{
    /* Physical memory backing */
    uint8_t* ram;               /* Main RAM pointer */
    uint64_t ram_size;          /* Total RAM size */

    /* Memory regions (for MMIO and special mappings) */
    venv_mem_region_t* regions;
    uint64_t region_count;
    uint64_t region_capacity;

    /* Page tables (future: for full MMU emulation) */
    void* page_tables;

    /* Statistics */
    uint64_t pages_allocated;
    uint64_t read_count;
    uint64_t write_count;
    uint64_t fetch_count;
};

typedef struct VenVMemoryContext venv_memory_t;

/*---- Memory Operations ----*/

/* Initialize memory context with given RAM size */
err_t venv_memory_init(venv_memory_t* mem, uint64_t ram_size);

/* Free all memory resources */
void venv_memory_destroy(venv_memory_t* mem);

/* Read a value from memory (generic size) */
err_t venv_memory_read(const venv_memory_t* mem, uint64_t addr, void* out_data, uint64_t size);

/* Write a value to memory (generic size) */
err_t venv_memory_write(venv_memory_t* mem, uint64_t addr, const void* data, uint64_t size);

/* Fetch an instruction (may have different alignment requirements) */
err_t venv_memory_fetch_insn(const venv_memory_t* mem, uint64_t addr, venv_insn_t* out_insn);

/* Read a 64-bit word */
static inline err_t venv_memory_read_u64(const venv_memory_t* mem, uint64_t addr, uint64_t* out_value)
{
    return venv_memory_read(mem, addr, out_value, sizeof(uint64_t));
}

/* Read a 32-bit word */
static inline err_t venv_memory_read_u32(const venv_memory_t* mem, uint64_t addr, uint32_t* out_value)
{
    return venv_memory_read(mem, addr, out_value, sizeof(uint32_t));
}

/* Read a 16-bit halfword */
static inline err_t venv_memory_read_u16(const venv_memory_t* mem, uint64_t addr, uint16_t* out_value)
{
    return venv_memory_read(mem, addr, out_value, sizeof(uint16_t));
}

/* Read an 8-bit byte */
static inline err_t venv_memory_read_u8(const venv_memory_t* mem, uint64_t addr, uint8_t* out_value)
{
    return venv_memory_read(mem, addr, out_value, sizeof(uint8_t));
}

/* Write a 64-bit word */
static inline err_t venv_memory_write_u64(venv_memory_t* mem, uint64_t addr, uint64_t value)
{
    return venv_memory_write(mem, addr, &value, sizeof(uint64_t));
}

/* Write a 32-bit word */
static inline err_t venv_memory_write_u32(venv_memory_t* mem, uint64_t addr, uint32_t value)
{
    return venv_memory_write(mem, addr, &value, sizeof(uint32_t));
}

/* Write a 16-bit halfword */
static inline err_t venv_memory_write_u16(venv_memory_t* mem, uint64_t addr, uint16_t value)
{
    return venv_memory_write(mem, addr, &value, sizeof(uint16_t));
}

/* Write an 8-bit byte */
static inline err_t venv_memory_write_u8(venv_memory_t* mem, uint64_t addr, uint8_t value)
{
    return venv_memory_write(mem, addr, &value, sizeof(uint8_t));
}

/* Check if an address is valid for access */
boolean venv_memory_is_valid_addr(const venv_memory_t* mem, uint64_t addr, uint64_t size, venv_mem_flags_t flags);

/* Map a memory-mapped I/O region */
err_t venv_memory_map_region(venv_memory_t* mem, uint64_t base, uint64_t size, venv_mem_flags_t flags);

/* Get direct pointer to RAM (for fast access when possible) */
static inline uint8_t* venv_memory_get_ram_ptr(venv_memory_t* mem, uint64_t addr)
{
    if (addr < mem->ram_size && mem->ram != NULL)
        return mem->ram + addr;
    return (uint8_t*)NULL;
}

#endif /* VenVMemory_h */
