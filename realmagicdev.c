/*****************************************************************************/

/*
 *     realmagicdev.c -- NetStream2000 EM8400 (quasar) driver
 *
 *      Copyright (C) 1999-2000  Sigma Designs
 *                    written by Emmanuel Michon <emmanuel_michon@sdesigns.com>
 *                           and Howard G. Page
 *					
 *      2.3.x/2.4.x compatibility by Thomas Sparr <thomas.sparr@kreatel.se>
 *      Rewritten for 2.6.x compatibility Sebastian Ortwein <kron@animeland.de>
 *      For 2.6.32+ compatibility Aaron <aaron619@126.com>
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

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>

#include <linux/pci.h>
#include <linux/interrupt.h>

#define CONFIG_MTRR 1

#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#include "realmagicdev.h"
#include "ioctl.h"
#include "registration.h"
#include "dma.h"

MODULE_AUTHOR("Sebastian Ortwein");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("support for sigma em84xx cards");
MODULE_SUPPORTED_DEVICE("em8400");

// NOTE: at the moment we only have support for one card
static struct em84xx em84xx_card;

static int realmagic_io_ioctl(struct inode *i_node, struct file *filp,
    unsigned int cmd, unsigned long arg)
{
    struct em84xx *card = filp->private_data;
    return realmagic_ioctl(card, cmd, arg);
}

static ssize_t realmagic_read(struct file *filp, char *buf, size_t count, loff_t *pos)
{
    return 0;
}

static int realmagic_mmap(struct file * filp, struct vm_area_struct * vma)
{
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    int i;

    switch (offset) {
        case REMAP_PIO:
            // for PIO space
            remap_pfn_range(vma, vma->vm_start,
                      em84xx_card.addr >> PAGE_SHIFT, vma->vm_end - vma->vm_start,
                      vma->vm_page_prot);

        break;

        case REMAP_DMA:
            // for DMA space

            // looking for the (only) dmabuf_table entry used but not yet mapped
            for (i = 0; i < MAX_DMABUF; i++)
                if (em84xx_card.dma[i].inuse&!em84xx_card.dma[i].ProcessVirtualAddress) break;

            if (i == MAX_DMABUF) {
                printk("MAX_dmabuf reached\n");
                return -EINVAL;
            }

            remap_pfn_range(vma, vma->vm_start,
                 em84xx_card.dma[i].PhysicalAddress >> PAGE_SHIFT, vma->vm_end - vma->vm_start,
                vma->vm_page_prot);

            em84xx_card.dma[i].ProcessVirtualAddress=vma->vm_start;

        break;
    default:
        return -EINVAL;
    }
    return 0; // never reached anyways
}

static int realmagic_open(struct inode *i_node, struct file *filp)
{
    int i;

    // init our card
    em84xx_card.ring0_irqcount = 0;
    em84xx_card.ring3_pid = 0;
    em84xx_card.sawEOS = 0;
    em84xx_card.irq.irq_status_register = 0;
    em84xx_card.irqstilltobeserviced = 0;

    for (i = 0; i < MAX_DMABUF; i++) {
        em84xx_card.dma[i].inuse = 0;
        em84xx_card.dma[i].Size = 0;
        em84xx_card.dma[i].ProcessVirtualAddress = 0;
        em84xx_card.dma[i].KernelVirtualAddress = 0;
        em84xx_card.dma[i].PhysicalAddress = 0;
    }

    filp->private_data = &em84xx_card;

    return 0;
}

static int realmagic_release(struct inode *i_node, struct file *filp)
{
    int i;
    em84xx_card.irq.irq_status_register = 0;

    for (i = 0; i < MAX_DMABUF; i++) {
        if (em84xx_card.dma[i].inuse) {
            if (em84xx_card.dma[i].ProcessVirtualAddress)
                printk("Trying to dmafree mmapped memory --- doing it anyways\n");

                dmafree(&em84xx_card, em84xx_card.dma[i].KernelVirtualAddress, em84xx_card.dma[i].Size);
                em84xx_card.dma[i].inuse = 0;
            }
        }
    return 0;
}

static struct file_operations realmagic_fops = {
    .owner =    THIS_MODULE,
    .read =     realmagic_read,
    .ioctl =    realmagic_io_ioctl,
    .mmap =     realmagic_mmap,
    .open =     realmagic_open,
    .release =  realmagic_release,
};

#define RISC_IRQ_REGISTER 0x1ffa
#define OLD_CLEAR_RISC 0x2
#define NEW_CLEAR_RISC 0x100

#define writePIO(b,thisregister) writel((b),(em84xx_card.mem+((thisregister)<<2)))
#define readPIO(thisregister) readl(em84xx_card.mem+((thisregister)<<2))

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t quasar_interrupt(int irq, void *dev_t, struct pt_regs * regs)
#else
static irqreturn_t quasar_interrupt(int irq, void *dev_t)
#endif
{
    static int otherirqcount = 0;
    long keepme=0; // initialized because gcc makes pointless ``might be used uninitialized''

    // in case of irq sharing, quick way out
    if (readPIO(RISC_IRQ_REGISTER) == 0) {
        otherirqcount++;
        return IRQ_HANDLED;
    }

    if (em84xx_card.quasar_dev_id >= 0x8470)
        writePIO(NEW_CLEAR_RISC, RISC_IRQ_REGISTER);
    else
        writePIO(OLD_CLEAR_RISC, RISC_IRQ_REGISTER);

    if (em84xx_card.irq.irq_status_register != 0)
        writePIO(0, em84xx_card.irq.irq_mask_register);

    em84xx_card.ring0_irqcount++;
    em84xx_card.irqstilltobeserviced++;

    // (playback) with 50000 this should log about every 20 minute
//    if (em84xx_card.ring0_irqcount%50000 == 0)
//        printk("[ring0 received %ld interrupts so far (skipped %d for other devices sharing line)]\n",
//            em84xx_card.ring0_irqcount, otherirqcount);

    // FIXME: what sould make this part?
    if (keepme & 0x1) {
        printk("EOS detected\n");
        em84xx_card.sawEOS = 1;
    }

    if (em84xx_card.ring3_pid) kill_pid(find_vpid(em84xx_card.ring3_pid), SIGUSR2, 0);

    // do not reenable interrupt: user mode does it for us.
    return IRQ_HANDLED;
}

static int realmagic_Init(struct pci_dev *dev, const struct pci_device_id *id)
{
    int result;
    unsigned char revision;

    em84xx_card.addr = pci_resource_start(dev, 0);
    em84xx_card.mapsize = pci_resource_len(dev, 0);
    em84xx_card.vendor = dev->subsystem_vendor;
    em84xx_card.device = dev->subsystem_device;
    em84xx_card.quasar_dev_id = id->device;
    em84xx_card.dev = dev;

    em84xx_card.mem = ioremap_nocache(em84xx_card.addr, em84xx_card.mapsize);

    if (em84xx_card.mem == NULL) {
        printk( KERN_ERR "em84xx: ioremap_nocache failed.\n" );
        // clean up irq
        free_irq(dev->irq, dev);
        return -EIO;
    }

    pci_set_master(dev);

    if (request_irq(dev->irq, quasar_interrupt, IRQF_SHARED, INTERRUPT_ENTRY_NAME, dev)) {
        printk( KERN_ERR "em84xx: IRQ %d not free.\n", dev->irq );
        return -EIO;
    }

    if ((result = pci_enable_device(dev)) != 0) {
        printk(KERN_ERR "em84xx: Unable to enable PCI device\n");
        return result;
    }

    pci_read_config_byte(dev, PCI_CLASS_REVISION, &revision);
    em84xx_card.pci_revision = revision;

    register_card(&em84xx_card);

    // print information about our card
    pr_info("em84xx: EM84xx %x (rev %d) ", dev->device, revision);
    printk("bus: %d, devfn: %d, irq: %d,", dev->bus->number, dev->devfn, dev->irq);
    printk("memory: 0x%08lx.\n", em84xx_card.addr);
    pr_info("em84xx: mapped-memory at 0x%p\n", em84xx_card.mem);
    printk("quasar_dev_id: #%d \n", em84xx_card.quasar_dev_id);

    return 0;
}

static void realmagic_DeInit( struct pci_dev *dev )
{
    unregister_card(&em84xx_card);

    iounmap(em84xx_card.mem);
    free_irq(dev->irq, dev);

    pci_disable_device(dev);
}

static struct pci_device_id realmagic_drv_tbl[] = {
   { VENDOR_ID_SIGMAD, DEVICE_ID_EM8400, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
   { VENDOR_ID_SIGMAD, DEVICE_ID_XCARD, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
   { 0, }
};

MODULE_DEVICE_TABLE(pci,realmagic_drv_tbl);
 
static struct pci_driver realmagic_drv = {
    .name      = INTERRUPT_ENTRY_NAME,
    .id_table  = realmagic_drv_tbl,
    .probe     = realmagic_Init,
    .remove    = realmagic_DeInit,
};

static int __init realmagic84xx_init(void)
{
    register_driver();

    if (register_chrdev(REALMAGIC_MAJOR, REALMAGIC_DEVICE_NAME, &realmagic_fops) == 0) {
        if( pci_register_driver(&realmagic_drv) == 0 )
            return 0;
        printk("em84xx: pci_module_init failed!\n");
        unregister_chrdev(REALMAGIC_MAJOR, REALMAGIC_DEVICE_NAME);
    } else
        printk("em84xx: register_chrdev failed!\n");

    return -EIO;  
}

static void __exit realmagic84xx_exit(void)
{
    pci_unregister_driver( &realmagic_drv );
    unregister_chrdev(REALMAGIC_MAJOR, REALMAGIC_DEVICE_NAME);

    unregister_driver();
}

module_init(realmagic84xx_init);
module_exit(realmagic84xx_exit);
