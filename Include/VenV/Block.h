#ifndef VenVBlock_h
#define VenVBlock_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Block Device
 * 
 * Provides:
 * - Virtual disk storage
 * - Backed by file on host (disk.img)
 * - Guest sees raw block device
 * - Guest OS provides filesystem (ext4, etc.)
 * - Memory-mapped interface for commands/data
 */

#include "../Core/Int.h"
#include "../Core/Error.h"
#include "Device.h"

/*---- Block Device Configuration ----*/

#define VENV_BLOCK_MMIO_SIZE    0x10000     /* 64KB MMIO region */
#define VENV_BLOCK_SECTOR_SIZE  512         /* Standard sector size */
#define VENV_BLOCK_MAX_SECTORS  256         /* Max sectors per request */
#define VENV_BLOCK_QUEUE_SIZE   32          /* Request queue size */

/*---- Block Device Registers (MMIO offsets) ----*/

#define VENV_BLOCK_CMD          0x0000      /* Command register */
#define VENV_BLOCK_STATUS       0x0008      /* Status register */
#define VENV_BLOCK_SECTOR_LO    0x0010      /* Starting sector (low 32 bits) */
#define VENV_BLOCK_SECTOR_HI    0x0018      /* Starting sector (high 32 bits) */
#define VENV_BLOCK_COUNT        0x0020      /* Number of sectors */
#define VENV_BLOCK_DATA_LO      0x0028      /* Data buffer address (low) */
#define VENV_BLOCK_DATA_HI      0x0030      /* Data buffer address (high) */
#define VENV_BLOCK_RESULT       0x0038      /* Command result */

/* Commands */
#define VENV_BLOCK_CMD_NOP      0x00        /* No operation */
#define VENV_BLOCK_CMD_READ     0x01        /* Read sectors */
#define VENV_BLOCK_CMD_WRITE    0x02        /* Write sectors */
#define VENV_BLOCK_CMD_FLUSH    0x03        /* Flush cache */
#define VENV_BLOCK_CMD_IDENTIFY 0x10        /* Identify device */

/* Status bits */
#define VENV_BLOCK_STATUS_READY     0x01    /* Device ready */
#define VENV_BLOCK_STATUS_BUSY      0x02    /* Command in progress */
#define VENV_BLOCK_STATUS_ERROR     0x04    /* Error occurred */
#define VENV_BLOCK_STATUS_IRQ_EN    0x08    /* Interrupt enable */

/* Identify data structure (returned by IDENTIFY command) */
struct VenVBlockIdentify
{
    uint64_t sector_count;      /* Total number of sectors */
    uint64_t sector_size;       /* Sector size in bytes */
    uint64_t max_sectors;       /* Max sectors per request */
    uint32_t version;           /* Interface version */
    uint8_t  model[32];         /* Model string */
    uint8_t  serial[32];        /* Serial number */
};

/*---- Block Request ----*/

struct VenVBlockRequest
{
    uint8_t cmd;                /* Command */
    uint64_t sector;            /* Starting sector */
    uint64_t count;             /* Number of sectors */
    uint64_t data_addr;         /* Guest memory address for data */
    boolean in_use;             /* Whether this slot is active */
};

typedef struct VenVBlockRequest venv_block_req_t;

/*---- Block Device State ----*/

struct VenVBlockState
{
    venv_device_t base;         /* Base device structure */
    
    /* Backing file info */
    char* disk_path;            /* Path to disk image file */
    void* disk_file;            /* Host file handle */
    uint64_t disk_size;         /* Size of disk in bytes */
    uint64_t sector_count;      /* Total sectors */
    
    /* Registers */
    uint64_t cmd;
    uint64_t status;
    uint64_t sector;
    uint64_t count;
    uint64_t data_addr;
    uint64_t result;
    
    /* Request queue */
    venv_block_req_t queue[VENV_BLOCK_QUEUE_SIZE];
    uint64_t queue_head;
    uint64_t queue_tail;
    
    /* Statistics */
    uint64_t read_count;
    uint64_t write_count;
    uint64_t error_count;
};

typedef struct VenVBlockState venv_block_t;

/*---- Block Device Operations ----*/

/* Initialize block device with backing file path */
err_t venv_block_init(venv_block_t* block, const char* disk_path);

/* Get block device structure */
venv_device_t* venv_block_get_device(venv_block_t* block);

/* Process pending block requests */
err_t venv_block_process(venv_block_t* block, void* memory_context);

/* Check if block device has pending interrupt */
boolean venv_block_irq_pending(venv_block_t* block);

/* Acknowledge block device interrupt */
err_t venv_block_ack_irq(venv_block_t* block);

/* Get disk size in bytes */
uint64_t venv_block_get_size(venv_block_t* block);

/* Get sector count */
uint64_t venv_block_get_sector_count(venv_block_t* block);

#endif /* VenVBlock_h */
