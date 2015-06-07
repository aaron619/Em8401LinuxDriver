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

#ifndef _DMA_H_
#define _DMA_H_

#include "realmagicdev.h"

unsigned long uvirt_to_phys(unsigned long adr);
void markpagereserved(unsigned long addr,size_t size);
void markpageunreserved(unsigned long addr,size_t size);

unsigned long dmaalloc(struct em84xx *card, size_t size);
void dmafree(struct em84xx *card, unsigned long addr, size_t size);

#endif /*_DMA_H_*/
