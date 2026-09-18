#ifndef VenVDevice_h
#define VenVDevice_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Device Architecture
 * 
 * Design goals:
 * - Well-defined interfaces between CPU and devices
 * - Devices are not tightly coupled to CPU implementation
 * - Easy to add new devices
 * - Memory-mapped I/O and/or port I/O support
 */

#include "../Core/Int.h"
#include "../Core/Error.h"
#include "ISA.h"

/*---- Device Operations ----*/

struct VenVDevice;
typedef struct VenVDevice venv_device_t;

/* Device operations table */
struct VenVDeviceOps
{
    /* Read from device (MMIO) */
    err_t (*read)(venv_device_t* dev, uint64_t offset, void* data, uint64_t size);
    
    /* Write to device (MMIO) */
    err_t (*write)(venv_device_t* dev, uint64_t offset, const void* data, uint64_t size);
    
    /* Update device state (called periodically) */
    err_t (*update)(venv_device_t* dev, uint64_t cycles);
    
    /* Reset device */
    err_t (*reset)(venv_device_t* dev);
    
    /* Destroy device */
    void  (*destroy)(venv_device_t* dev);
};

typedef struct VenVDeviceOps venv_device_ops_t;

/* Base device structure */
struct VenVDevice
{
    const char* name;           /* Device name */
    void* private_data;         /* Device-specific data */
    venv_device_ops_t* ops;     /* Operations table */
    
    /* Interrupt line (for signaling CPU) */
    boolean irq_asserted;
    int irq_line;
    
    /* Back-pointer to VM (for accessing CPU/memory) */
    void* vm;
};

/*---- Device Registration ----*/

err_t venv_device_register(void* vm, venv_device_t* dev, uint64_t mmio_base, uint64_t mmio_size);
err_t venv_device_unregister(void* vm, venv_device_t* dev);

/*---- Helper macros for device implementation ----*/

#define VENV_DEVICE_READ_U8(dev, offset) \
    ({ uint8_t val; (dev)->ops->read((dev), (offset), &val, sizeof(val)); val; })

#define VENV_DEVICE_WRITE_U8(dev, offset, val) \
    (dev)->ops->write((dev), (offset), &(val), sizeof(val))

#define VENV_DEVICE_READ_U32(dev, offset) \
    ({ uint32_t val; (dev)->ops->read((dev), (offset), &val, sizeof(val)); val; })

#define VENV_DEVICE_WRITE_U32(dev, offset, val) \
    (dev)->ops->write((dev), (offset), &(val), sizeof(val))

#endif /* VenVDevice_h */
