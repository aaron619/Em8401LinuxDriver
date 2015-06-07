/*****************************************************************************/

/*
 *     dma.h -- NetStream2000 EM8400 (quasar) driver
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
#include <linux/sched.h>
#include <linux/pagemap.h>
#include <asm/io.h>

#include "dma.h"
//extern struct mm_struct *get_task_mm(struct task_struct *task);


unsigned long uvirt_to_phys(unsigned long adr)
{
    unsigned long ret = 0UL;
    pmd_t *pmd;
    pte_t *ptep, pte;
    pgd_t *pgd = pgd_offset(current->mm, adr);

    if (!pgd_none(*pgd)) {
        pmd = pmd_offset((void *)pgd, adr);
        if (!pmd_none(*pmd)) {
            ptep = pte_offset_kernel(pmd, adr);
            pte = *ptep;
            if (pte_present(pte)) {
                unsigned long kva;
                kva = ((unsigned long)(page_address(pte_page(pte)))
                    |((unsigned long)(adr&(PAGE_SIZE-1))));
                    ret = virt_to_phys((void*) kva);
            }
        }
    }
    return ret;
}

void markpagereserved(unsigned long addr,size_t size)
{
    struct page *page;

    for (page = virt_to_page(addr); page < virt_to_page(addr+size); page++)
        SetPageReserved(page);
}

void markpageunreserved(unsigned long addr,size_t size)
{
    struct page *page;

    for (page = virt_to_page(addr); page < virt_to_page(addr+size); page++)
        ClearPageReserved(page);
}

unsigned long dmaalloc(struct em84xx *card, size_t size)
{
    unsigned long addr = __get_dma_pages(GFP_KERNEL, get_order(size));

    printk("em84xx: dmaalloc called with %d ",size);

    if (!addr) {
        printk("(refused)\n");
    } else {
        markpagereserved(addr, size); // set as not-swappable among other things
        printk("(ok at %p)\n", (char *)addr);
        return addr;
    }

    return 0;
}

void dmafree(struct em84xx *card, unsigned long addr, size_t size)
{
    printk("dmafree called with %p, %d\n",(char *)addr,size);

    if (size > 0) {
        markpageunreserved(addr, size);
        free_pages(addr, get_order(size));
    }
}
