/*****************************************************************************/

/*
 *     ioctl.c -- NetStream2000 EM8400 (quasar) driver
 *
 *      Copyright (C) 2006 Christian Gmeiner
 *
 *      2.3.x/2.4.x compatibility by Thomas Sparr <thomas.sparr@kreatel.se>
 *      Rewritten for 2.6.x compatibility Sebastian Ortwein <kron@animeland.de>
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

#define __NO_VERSION__
#include <asm/uaccess.h>
#include <asm/io.h>

#include "ioctl.h"
#include "dma.h"

int realmagic_ioctl(struct em84xx* card, unsigned int cmd,
    unsigned long arg) {

    int i;
    struct dmabuf_element d;
    unsigned long processvirtual, physical;

    switch (cmd) {
        case IOCTL_GET_PIOMAPSIZE:
            if (copy_to_user((void *) arg, &card->mapsize, sizeof(unsigned long))) {
                return -EFAULT;
            }
        break;

        case IOCTL_DMABUF_ALLOC:

            for (i = 0; i < MAX_DMABUF; i++)
                if (!card->dma[i].inuse) break;

            if (i == MAX_DMABUF) {
                printk("MAX_dmabuf reached\n");
                return -EINVAL;
            }

            if (copy_from_user(&card->dma[i],(void *)arg, sizeof(struct dmabuf_element))) {
                return -EFAULT;
            }

            card->dma[i].KernelVirtualAddress = (unsigned long)dmaalloc(card, card->dma[i].Size);

            // dmaalloc failure?
            if (card->dma[i].KernelVirtualAddress == 0)
                return -EINVAL;

            card->dma[i].PhysicalAddress = virt_to_phys((char *)card->dma[i].KernelVirtualAddress);
            card->dma[i].inuse = 1;
            card->dma[i].ProcessVirtualAddress = 0;

            if (copy_to_user((void *)arg, &card->dma[i],sizeof(struct dmabuf_element))) {
                return -EFAULT;
            }

        break;

        case IOCTL_DMABUF_FREE:
            /* the user space code must call munmap and put zero in ProcessVirtualAddress
            field before getting here */

            if (copy_from_user(&d,(void *)arg, sizeof(struct dmabuf_element))) {
                return -EFAULT;
            }

            // looking for the (only) index using KernelVirtualAddress
            for (i = 0; i < MAX_DMABUF; i++)
                if (card->dma[i].inuse && 
                   (card->dma[i].KernelVirtualAddress == d.KernelVirtualAddress)) break;

            if (i == MAX_DMABUF) {
                printk("MAX_dmabuf reached\n");
                return -EINVAL;
            }

            if (d.ProcessVirtualAddress) {
                printk("Trying to dmafree mmapped memory\n");
                return -EINVAL;
            } else {
                dmafree(card, card->dma[i].KernelVirtualAddress, card->dma[i].Size);
                card->dma[i].inuse = 0;
            }

            if (copy_to_user((void *)arg, &card->dma[i],sizeof(struct dmabuf_element))) {
                return -EFAULT;
            }
        break;

        case IOCTL_TRANSLATE_PROCESSVIRTUAL_TO_PHYSICAL:

            if (copy_from_user(&processvirtual,(void *)arg, sizeof(unsigned long))) {
                return -EFAULT;
            }

            physical = (unsigned long)uvirt_to_phys(processvirtual);

            if (copy_to_user((void *)arg,&physical, sizeof(unsigned long))) {
                return -EFAULT;
            }
        break;

        case IOCTL_UNRESERVE_PAGE_BY_PHYSICAL_ADDRESS:

            if (copy_from_user(&physical,(void *)arg,sizeof(unsigned long))) {
                return -EFAULT;
            }
            markpageunreserved((unsigned long)phys_to_virt(physical), PAGE_SIZE);
        break;

        case IOCTL_RESERVE_PAGE_BY_UV:

            if (copy_from_user(&processvirtual,(void *)arg,sizeof(unsigned long))) {
                return -EFAULT;
            }

            physical = (unsigned long)uvirt_to_phys(processvirtual);
            markpagereserved((unsigned long)phys_to_virt(physical),PAGE_SIZE);
        break;

        case IOCTL_UNRESERVE_PAGE_BY_UV:

        if (copy_from_user(&processvirtual,(void *)arg,sizeof(unsigned long))) {
                return -EFAULT;
            }

            physical = (unsigned long)uvirt_to_phys(processvirtual);
            markpageunreserved((unsigned long)phys_to_virt(physical),PAGE_SIZE);
        break;

        case IOCTL_SEND_RING3_PID:
            if (copy_from_user(&card->ring3_pid, (void *)arg, sizeof(unsigned long))) {
                return -EFAULT;
            }
            printk("[Received ring3 pid is %ld]\n", card->ring3_pid);
        break;

        case IOCTL_SEND_IRQ_DATA:
            if (copy_from_user(&card->irq, (void *)arg, sizeof(struct irq_data))) {
                return -EFAULT;
            }

            printk("[Received irq data: %08lx-%08lx-%08lx]\n",
                card->irq.irq_status_register,
                card->irq.irq_mask_register,
                card->irq.irq_status
                );
        break;

        case IOCTL_GET_IRQ_COUNT:
            if (copy_to_user((void *)arg, &card->ring0_irqcount, sizeof(unsigned long))) {
                return -EFAULT;
            }
        break;

        case IOCTL_GET_PCI_ID_SUBSYSTEM_VENDOR:
            if (copy_to_user((void *) arg, &card->vendor, sizeof(unsigned short))) {
                return -EFAULT;
            }
        break;

        case IOCTL_GET_PCI_ID_SUBSYSTEM_DEVICE:
            if (copy_to_user((void *) arg, &card->device, sizeof(unsigned short))) {
                return -EFAULT;
            }
        break;

        case IOCTL_GET_PCI_DEVICE_ID:
            if (copy_to_user((void *) arg, &card->quasar_dev_id, sizeof(unsigned short))) {
                return -EFAULT;
            }
        break;

        case IOCTL_GET_SAWEOS:
            if (copy_to_user((void *)arg, &card->sawEOS, sizeof(unsigned long))) {
                return -EFAULT;
            }

            if (card->sawEOS == 1) {
                card->sawEOS = 0;
                printk("Resetting EOS detection\n");
            }
        break;

        case IOCTL_FORGE_IRQ:
            card->irqstilltobeserviced++;
        break;

        default:
            return -EINVAL;
    }

    return 0; // never reached anyways
}
