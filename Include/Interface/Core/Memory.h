#ifndef CoreMemory_h
#define CoreMemory_h
#include "../../Pure.h"

#include "Byte.h"
#include "Error.h"

err_t memory_copy (voidptr_t from, voidptr_t to, uint64_t size);

static inline void memory_set(voidptr_t ptr, uint8_t value, uint64_t size);
static inline voidptr_t memory_alignPointer   (voidptr_t ptr, uint64_t alignment);
static inline uint64_t  memory_alignForward   (uint64_t address, uint64_t alignment);
static inline uint64_t  memory_nextPowerOfTwo (uint64_t bytes);

static inline void
memory_set(voidptr_t ptr, uint8_t value, uint64_t size)
{
    if (ptr == NULL) return;
    uint8_t* p = (uint8_t*)ptr;
    uint64_t i = 0;
    while (i < size)
    {
        p[i] = value;
        i++;
    }
}

static inline uint64_t
memory_alignForward(uint64_t address, uint64_t alignment)
{
	return (address + (alignment - 1)) & ~(alignment - 1);
}

static inline voidptr_t
memory_alignPointer(voidptr_t ptr, uint64_t alignment)
{
	return (voidptr_t)memory_alignForward((uint64_t)ptr, alignment);
}

static inline uint64_t
memory_nextPowerOfTwo(uint64_t bytes)
{
	if (bytes <= 1) return 1;

	bytes--;
	bytes |= bytes >> 1;
	bytes |= bytes >> 2;
	bytes |= bytes >> 4;
	bytes |= bytes >> 8;
	bytes |= bytes >> 16;
	bytes |= bytes >> 32;
	bytes++;

	return bytes;
}

/*---- ----  ---- [ Memory pool ] ----  ---- ----*/

struct MemoryPoolBlock
{
	boolean is_allocated;
	uint64_t size;
	struct MemoryPoolBlock* next;
};

struct MemoryPoolHeader
{
	uint64_t block_count;
	uint64_t size;
	struct MemoryPoolBlock* first_block;
};

#define CORE_MEMORY_DEFAULT_ALIGNMENT 8

struct MemoryPoolHeader* memorypool_initialize(byte_t* buffer, uint64_t size_of_buffer);

err_t memorypool_allocate        (struct MemoryPoolHeader* header, uint64_t bytes, voidptr_t* pointer);
err_t memorypool_allocateAligned (struct MemoryPoolHeader* header, uint64_t bytes, uint64_t alignment, voidptr_t* out_pointer);
err_t memorypool_reallocate      (struct MemoryPoolHeader* header, uint64_t bytes, voidptr_t* pointer);
err_t memorypool_deallocate      (struct MemoryPoolHeader* header, voidptr_t pointer);

voidptr_t mempool_realloc (struct MemoryPoolHeader* header, voidptr_t pointer, uint64_t bytes, err_t* occured);
voidptr_t mempool_alloc   (struct MemoryPoolHeader* header, uint64_t bytes, err_t* occured);
void      mempool_dealloc (struct MemoryPoolHeader* header, voidptr_t pointer, err_t* occured);

#define mpool_allocateType(H, T, P) \
	memorypool_allocateAligned((H), sizeof(T), _Alignof(T), (voidptr_t*)((P)))

#define CORE_MEMORY_SELECT(_1, _2, _3, _4, NAME, ...) NAME

#define CORE_MPOOL_ALLOC_2(header, bytes) \
	mpool_alloc((header), (bytes), NULL)

#define CORE_MPOOL_ALLOC_3(header, bytes, occured) \
	mpool_alloc((header), (bytes), (occured))

#define mpool_alloc(...) \
	CORE_MEMORY_SELECT(__VA_ARGS__, \
		CORE_MPOOL_ALLOC_3, \
		CORE_MPOOL_ALLOC_2 \
	)(__VA_ARGS__)

#define CORE_MPOOL_REALLOC_3(header, bytes, out_pointer) \
	mpool_realloc((header), (bytes), (out_pointer), NULL)

#define CORE_MPOOL_REALLOC_4(header, bytes, out_pointer, occured) \
	mpool_realloc((header), (bytes), (out_pointer), (occured))

#define mpool_realloc(...) \
	CORE_MEMORY_SELECT(__VA_ARGS__, \
		CORE_MPOOL_REALLOC_4, \
		CORE_MPOOL_REALLOC_3 \
	)(__VA_ARGS__)

#define CORE_MPOOL_DEALLOC_2(header, pointer) \
	mpool_dealloc((header), (pointer), NULL)

#define CORE_MPOOL_DEALLOC_3(header, pointer, occured) \
	mpool_dealloc((header), (pointer), (occured))

#define mpool_dealloc(...) \
	CORE_MEMORY_SELECT(__VA_ARGS__, \
		CORE_MPOOL_DEALLOC_3, \
		CORE_MPOOL_DEALLOC_2 \
	)(__VA_ARGS__)

#endif // CoreMemory_h