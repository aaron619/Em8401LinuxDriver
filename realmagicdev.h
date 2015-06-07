/*****************************************************************************/

/*
 *     realmagicdev.h -- NetStream2000 EM8400 (quasar) driver
 *
 *      Copyright (C) 1999-2000  Sigma Designs
 *                    written by Emmanuel Michon <emmanuel_michon@sdesigns.com>
 *                           and Howard G. Page
 *
 *      2.3.x/2.4.x compatibility by Thomas Sparr <thomas.sparr@kreatel.se>
 *      Rewritten for 2.6.x compatibility Sebastian Ortwein <kron@animeland.de>
 *		For 2.6.32+ compatibility Aaron <aaron619@126.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#ifndef _REALMAGICDEV_H_
#define _REALMAGICDEV_H_

#include <linux/version.h>

#define REALMAGIC_DEVICE_NAME 			"realmagic"
#define INTERRUPT_ENTRY_NAME 			"quasar"
#define REALMAGIC_MAJOR 			122

// mapping primitives
#define REMAP_PIO				0x1000
#define REMAP_DMA				0x2000

#define IOCTL_GET_PIOMAPSIZE			0x1233
#define IOCTL_DMABUF_ALLOC			0x1234
#define IOCTL_DMABUF_FREE			0x1235
#define IOCTL_TRANSLATE_PROCESSVIRTUAL_TO_PHYSICAL	0x1236
#define IOCTL_UNRESERVE_PAGE_BY_PHYSICAL_ADDRESS	0x1237

#define IOCTL_SEND_RING3_PID			0x1238
#define IOCTL_SEND_IRQ_DATA			0x1239

// check interrupt to signal translation consistency
#define IOCTL_GET_IRQ_COUNT 			0x123c
#define IOCTL_GET_PCI_ID_SUBSYSTEM_VENDOR	0x123d
#define IOCTL_GET_PCI_ID_SUBSYSTEM_DEVICE	0x123e
#define IOCTL_FORGE_IRQ				0x123f
#define IOCTL_GET_PCI_DEVICE_ID			0x1240
#define IOCTL_GET_SAWEOS			0x1241

#define IOCTL_RESERVE_PAGE_BY_UV		0x12fe // dtvro patch
#define IOCTL_UNRESERVE_PAGE_BY_UV		0x12ff

#define MAX_DMABUF 40

#define VENDOR_ID_SIGMAD			0x1105
#define DEVICE_ID_EM8400			0x8400
#define DEVICE_ID_XCARD				0x8475

struct dmabuf_element {
    int inuse;
    unsigned long Size;
    unsigned long ProcessVirtualAddress;
    unsigned long KernelVirtualAddress;
    unsigned long PhysicalAddress;
};

struct irq_data {
    /* if risc_status_register is zero, it means the struct has not been
       initialized. */
    unsigned long irq_status_register;
    unsigned long irq_mask_register;

    // some values.
    unsigned long irq_status;
};

struct em84xx {
    /* pci values */
    int pci_revision;
    unsigned short vendor;
    unsigned short device;
    unsigned short quasar_dev_id;

    /* memory mapping */
    struct pci_dev *dev;
    unsigned long addr;
    void* mem;
    unsigned long mapsize;

    /* DMA */
    struct dmabuf_element dma[MAX_DMABUF];

    /* some stuff */
    unsigned long ring0_irqcount;
    unsigned long ring3_pid;
    unsigned long sawEOS;
    unsigned long irqstilltobeserviced;

    /* irq */
    struct irq_data irq;

    /* Sysfs */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,46)
    struct class_device *classdev_0;
    struct class_device *classdev_1;
    struct class_device *classdev_2;
    struct class_device *classdev_3;
#endif
};

#endif /*_REALMAGICDEV_H_*/
