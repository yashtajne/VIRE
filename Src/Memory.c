/*
 * VenV - Virtual Environment Engine
 * Memory Implementation
 */

#include "../Include/VenV/Memory.h"
#include "../Include/VenV/ISA.h"
#include "../Include/VenV/Device.h"
#include "../Include/Interface/Std/Memory.h"
#include "../Include/Pure.h"

/*---- Memory Initialization ----*/

err_t venv_memory_init(venv_memory_t* mem, uint64_t ram_size)
{
    if (mem == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Initialize structure */
    mem->ram = NULL;
    mem->ram_size = ram_size;
    mem->regions = NULL;
    mem->region_count = 0;
    mem->region_capacity = 0;
    mem->page_tables = NULL;
    mem->pages_allocated = 0;
    mem->read_count = 0;
    mem->write_count = 0;
    mem->fetch_count = 0;

    /* Allocate RAM */
    err_t err = heap_allocate(ram_size, (voidptr_t*)&mem->ram);
    if (err != PURE_OK)
        return err;

    /* Zero-initialize RAM */
    for (uint64_t i = 0; i < ram_size; i++)
        mem->ram[i] = 0;

    return PURE_OK;
}

/*---- Memory Destruction ----*/

void venv_memory_destroy(venv_memory_t* mem)
{
    if (mem == NULL)
        return;

    /* Free RAM */
    if (mem->ram != NULL)
    {
        heap_deallocate(mem->ram);
        mem->ram = NULL;
    }

    /* Free MMIO regions */
    if (mem->regions != NULL)
    {
        for (uint64_t i = 0; i < mem->region_count; i++)
        {
            if (mem->regions[i].data != NULL)
                heap_deallocate(mem->regions[i].data);
        }
        heap_deallocate(mem->regions);
        mem->regions = NULL;
    }

    mem->ram_size = 0;
    mem->region_count = 0;
    mem->region_capacity = 0;
}

/*---- Memory Access ----*/

err_t venv_memory_read(const venv_memory_t* mem, uint64_t addr, void* out_data, uint64_t size)
{
    if (mem == NULL || out_data == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Check if address is in RAM */
    if (addr + size > mem->ram_size)
    {
        /* Check MMIO regions */
        for (uint64_t i = 0; i < mem->region_count; i++)
        {
            if (mem->regions[i].is_mapped &&
                addr >= mem->regions[i].base &&
                addr + size <= mem->regions[i].base + mem->regions[i].size)
            {
                /* MMIO read - not implemented yet */
                return PURE_ERROR_FILE_READ_FAILED;
            }
        }

        return PURE_ERROR_OUT_OF_BOUNDS;
    }

    /* Read from RAM */
    uint8_t* src = mem->ram + addr;
    uint8_t* dst = (uint8_t*)out_data;

    for (uint64_t i = 0; i < size; i++)
        dst[i] = src[i];

    ((venv_memory_t*)mem)->read_count++;

    return PURE_OK;
}

err_t venv_memory_write(venv_memory_t* mem, uint64_t addr, const void* data, uint64_t size)
{
    if (mem == NULL || data == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Check if address is in RAM */
    if (addr + size > mem->ram_size)
    {
        /* Check MMIO regions */
        for (uint64_t i = 0; i < mem->region_count; i++)
        {
            if (mem->regions[i].is_mapped &&
                addr >= mem->regions[i].base &&
                addr + size <= mem->regions[i].base + mem->regions[i].size)
            {
                /* MMIO write - delegate to device */
                if (mem->regions[i].device != NULL &&
                    mem->regions[i].device->ops != NULL &&
                    mem->regions[i].device->ops->write != NULL)
                {
                    uint64_t offset = addr - mem->regions[i].base;
                    return mem->regions[i].device->ops->write(
                        mem->regions[i].device, offset, data, size);
                }
                return PURE_ERROR_FILE_WRITE_FAILED;
            }
        }

        return PURE_ERROR_OUT_OF_BOUNDS;
    }

    /* Write to RAM */
    uint8_t* dst = mem->ram + addr;
    const uint8_t* src = (const uint8_t*)data;

    for (uint64_t i = 0; i < size; i++)
        dst[i] = src[i];

    mem->write_count++;

    return PURE_OK;
}

err_t venv_memory_fetch_insn(const venv_memory_t* mem, uint64_t addr, venv_insn_t* out_insn)
{
    if (mem == NULL || out_insn == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Check alignment */
    if (addr & 0x3)
        return PURE_ERROR_INVALID_ARGUMENT;

    /* Check bounds */
    if (addr + sizeof(venv_insn_t) > mem->ram_size)
        return PURE_ERROR_OUT_OF_BOUNDS;

    /* Fetch instruction (little-endian) */
    uint8_t* ptr = mem->ram + addr;
    *out_insn = (venv_insn_t)ptr[0] |
                ((venv_insn_t)ptr[1] << 8) |
                ((venv_insn_t)ptr[2] << 16) |
                ((venv_insn_t)ptr[3] << 24);

    ((venv_memory_t*)mem)->fetch_count++;

    return PURE_OK;
}

/*---- Address Validation ----*/

boolean venv_memory_is_valid_addr(const venv_memory_t* mem, uint64_t addr, uint64_t size, venv_mem_flags_t flags)
{
    if (mem == NULL)
        return false;

    /* Check for overflow */
    if (addr + size < addr)
        return false;

    /* Check RAM region */
    if (addr < mem->ram_size && addr + size <= mem->ram_size)
    {
        /* TODO: Check protection flags when implemented */
        return true;
    }

    /* Check MMIO regions */
    for (uint64_t i = 0; i < mem->region_count; i++)
    {
        if (mem->regions[i].is_mapped &&
            addr >= mem->regions[i].base &&
            addr + size <= mem->regions[i].base + mem->regions[i].size)
        {
            /* Check access flags */
            if ((flags & VENV_MEM_READ) && !(mem->regions[i].flags & VENV_MEM_READ))
                return false;
            if ((flags & VENV_MEM_WRITE) && !(mem->regions[i].flags & VENV_MEM_WRITE))
                return false;
            return true;
        }
    }

    return false;
}

/*---- MMIO Region Mapping ----*/

err_t venv_memory_map_region(venv_memory_t* mem, uint64_t base, uint64_t size, venv_mem_flags_t flags)
{
    if (mem == NULL)
        return PURE_ERROR_NULL_POINTER;

    /* Check if we need to grow the regions array */
    if (mem->region_count >= mem->region_capacity)
    {
        uint64_t new_capacity = mem->region_capacity == 0 ? 8 : mem->region_capacity * 2;
        venv_mem_region_t* new_regions = NULL;

        err_t err = heap_allocate(new_capacity * sizeof(venv_mem_region_t),
                                  (voidptr_t*)&new_regions);
        if (err != PURE_OK)
            return err;

        /* Copy existing regions */
        if (mem->regions != NULL)
        {
            for (uint64_t i = 0; i < mem->region_count; i++)
                new_regions[i] = mem->regions[i];

            heap_deallocate(mem->regions);
        }

        mem->regions = new_regions;
        mem->region_capacity = new_capacity;
    }

    /* Add new region */
    venv_mem_region_t* region = &mem->regions[mem->region_count];
    region->base = base;
    region->size = size;
    region->data = NULL;  /* MMIO regions don't have backing data */
    region->flags = flags;
    region->is_mapped = true;

    mem->region_count++;

    return PURE_OK;
}
