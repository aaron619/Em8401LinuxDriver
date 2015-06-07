/*****************************************************************************/

/*
 *     registration.c -- NetStream2000 EM8400 (quasar) driver
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

#include "registration.h"
#include "devfs.h"
#include "udev.h"

static struct registrar_s *registrars[] = {
    &devfs_registrar,
    &udev_registrar,
    NULL
};

void register_driver(void) {
    int i;
    for (i = 0; registrars[i]; i++) {
        if (registrars[i]->register_driver)
            registrars[i]->register_driver();
    }
}

void unregister_driver(void) {
    int i;
    for (i = 0; registrars[i]; i++) {
        if (registrars[i]->unregister_driver)
            registrars[i]->unregister_driver();
   }
}

void register_card(struct em84xx* card) {
    int i;
    for (i = 0; registrars[i]; i++) {
        if (registrars[i]->register_card)
            registrars[i]->register_card(card);
   }
}

void unregister_card(struct em84xx* card) {
    int i;
    for (i = 0; registrars[i]; i++) {
        if (registrars[i]->unregister_card)
            registrars[i]->unregister_card(card);
   }
}
