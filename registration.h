/*****************************************************************************/

/*
 *     registration.h -- NetStream2000 EM8400 (quasar) driver
 *
 *      Copyright (C) 2006 Christian Gmeiner
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

#ifndef _REGISTRATION_H_
#define _REGISTRATION_H_

#include <linux/types.h>
#include <linux/version.h>

#include "realmagicdev.h"

struct registrar_s {
    void (*register_driver)(void);
    void (*unregister_driver)(void);
    void (*register_card)(struct em84xx* card);
    void (*unregister_card)(struct em84xx* card);
};

extern void register_driver(void);
extern void unregister_driver(void);
extern void register_card(struct em84xx* card);
extern void unregister_card(struct em84xx* card);

#endif /*_REGISTRATION_H_*/
