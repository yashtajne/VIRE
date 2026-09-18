#ifndef VenVNet_h
#define VenVNet_h

/*
 * VenV - Virtual Environment Engine
 * Virtual Network Interface Card (NIC)
 * 
 * Provides:
 * - Virtual network interface for guest OS
 * - Guest OS provides TCP/IP stack, sockets, etc.
 * - Engine only emulates hardware device
 * - Host-side bridge connects to real network
 * - Supports port forwarding
 * 
 * Design: Simple descriptor-based interface
 * (Similar to virtio-net but simplified)
 */

#include "../Core/Int.h"
#include "../Core/Error.h"
#include "Device.h"

/*---- NIC Configuration ----*/

#define VENV_NET_MMIO_SIZE      0x10000     /* 64KB MMIO region */
#define VENV_NET_RX_QUEUE_SIZE  256         /* Receive queue entries */
#define VENV_NET_TX_QUEUE_SIZE  256         /* Transmit queue entries */
#define VENV_NET_MTU            1500        /* Max transmission unit */
#define VENV_NET_MAX_PKT_SIZE   (VENV_NET_MTU + 64)  /* Max packet size with overhead */
#define VENV_NET_MAC_LEN        6           /* MAC address length */

/*---- NIC Registers (MMIO offsets) ----*/

#define VENV_NET_CTRL           0x0000      /* Control register */
#define VENV_NET_STATUS         0x0008      /* Status register */
#define VENV_NET_RX_DESC_LO     0x0010      /* RX descriptor ring base (low) */
#define VENV_NET_RX_DESC_HI     0x0018      /* RX descriptor ring base (high) */
#define VENV_NET_TX_DESC_LO     0x0020      /* TX descriptor ring base (low) */
#define VENV_NET_TX_DESC_HI     0x0028      /* TX descriptor ring base (high) */
#define VENV_NET_RX_IDX         0x0030      /* RX consumer index */
#define VENV_NET_TX_IDX         0x0038      /* TX producer index */
#define VENV_NET_MAC0           0x0040      /* MAC address bytes 0-3 */
#define VENV_NET_MAC1           0x0044      /* MAC address bytes 4-5 */
#define VENV_NET_INT_MASK       0x0050      /* Interrupt mask */

/* Control bits */
#define VENV_NET_CTRL_RESET     0x01        /* Reset device */
#define VENV_NET_CTRL_ENABLE    0x02        /* Enable device */
#define VENV_NET_CTRL_RX_EN     0x04        /* Enable receiver */
#define VENV_NET_CTRL_TX_EN     0x08        /* Enable transmitter */

/* Status bits */
#define VENV_NET_STATUS_LINK_UP 0x01        /* Link is up */
#define VENV_NET_STATUS_RX_IRQ  0x02        /* RX interrupt pending */
#define VENV_NET_STATUS_TX_IRQ  0x04        /* TX interrupt pending */

/*---- Descriptor Format ----*/

struct VenVNetDesc
{
    uint64_t addr;              /* Buffer address (guest physical) */
    uint32_t len;               /* Buffer length */
    uint32_t flags;             /* Descriptor flags */
};

/* Descriptor flags */
#define VENV_NET_DESC_USED      0x0001      /* Device has used this descriptor */
#define VENV_NET_DESC_NEXT      0x0002      /* Next descriptor in chain */
#define VENV_NET_DESC_WRITE     0x0004      /* Writeable by device */

/*---- Packet Header (prepended to each packet) ----*/

struct VenVNetPktHeader
{
    uint16_t pkt_type;          /* Packet type */
    uint16_t flags;             /* Flags */
    uint32_t len;               /* Packet length */
};

typedef struct VenVNetPktHeader venv_net_pkt_hdr_t;

/*---- NIC State ----*/

struct VenVNetState
{
    venv_device_t base;         /* Base device structure */
    
    /* Device state */
    uint64_t control;
    uint64_t status;
    uint64_t rx_desc_base;
    uint64_t tx_desc_base;
    uint64_t rx_idx;
    uint64_t tx_idx;
    uint64_t int_mask;
    
    /* MAC address */
    uint8_t mac[VENV_NET_MAC_LEN];
    
    /* Descriptor rings (cached copies) */
    struct VenVNetDesc rx_queue[VENV_NET_RX_QUEUE_SIZE];
    struct VenVNetDesc tx_queue[VENV_NET_TX_QUEUE_SIZE];
    
    /* Queue indices */
    uint64_t rx_avail;          /* Available RX descriptors */
    uint64_t rx_used;           /* Used RX descriptors */
    uint64_t tx_avail;          /* Available TX descriptors */
    uint64_t tx_used;           /* Used TX descriptors */
    
    /* Statistics */
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t tx_packets;
    uint64_t tx_bytes;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t dropped;
    
    /* Host-side networking */
    void* host_nic;             /* Host network interface handle */
    boolean link_up;
};

typedef struct VenVNetState venv_net_t;

/*---- NIC Operations ----*/

/* Initialize NIC with random MAC or specified MAC */
err_t venv_net_init(venv_net_t* net, const uint8_t* mac_addr);

/* Get NIC device structure */
venv_device_t* venv_net_get_device(venv_net_t* net);

/* Process TX queue (send packets to host) */
err_t venv_net_tx_process(venv_net_t* net, void* memory_context);

/* Process RX queue (receive packets from host) */
err_t venv_net_rx_process(venv_net_t* net, void* memory_context);

/* Send packet from host to guest (for incoming network data) */
err_t venv_net_receive(venv_net_t* net, const uint8_t* data, uint64_t len);

/* Check if NIC has pending interrupt */
boolean venv_net_irq_pending(venv_net_t* net);

/* Acknowledge NIC interrupt */
err_t venv_net_ack_irq(venv_net_t* net);

/* Set link state */
err_t venv_net_set_link(venv_net_t* net, boolean up);

/* Get MAC address */
const uint8_t* venv_net_get_mac(venv_net_t* net);

#endif /* VenVNet_h */
