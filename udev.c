/*****************************************************************************/

/*
 *     udev.c -- NetStream2000 EM8400 (quasar) driver
 *
 *      Copyright (C) 2006 Christian Gmeiner
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *		For 2.6.32+ compatibility Aaron <aaron619@126.com>	
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

#include "udev.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,2)

#include <linux/device.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
#define class class_simple
#define class_create class_simple_create
#define class_device_create class_simple_device_add
#define class_device_destroy(cls, dev) class_simple_device_remove(dev)
#define class_destroy class_simple_destroy
#endif

struct class* em84xx_class;

static void udev_register_driver(void) {
    em84xx_class = class_create(THIS_MODULE, "em84xx");
}

static void udev_unregister_driver(void) {
    class_destroy(em84xx_class);
}

static void udev_register_card(struct em84xx* card) {

    card->classdev_0 = device_create(em84xx_class,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
                       NULL,
#endif
                       MKDEV(REALMAGIC_MAJOR, 0),
                       &card->dev->dev,
                       "%s%d",
                       REALMAGIC_DEVICE_NAME, 0);

    card->classdev_1 = device_create(em84xx_class,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
                       NULL,
#endif
                       MKDEV(REALMAGIC_MAJOR, 1),
                       &card->dev->dev,
                       "%s%d",
                       REALMAGIC_DEVICE_NAME, 1);

    card->classdev_2 = device_create(em84xx_class,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
                       NULL,
#endif
                       MKDEV(REALMAGIC_MAJOR, 2),
                       &card->dev->dev,
                       "%s%d",
                       REALMAGIC_DEVICE_NAME, 2);

    card->classdev_3 = device_create(em84xx_class,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
                       NULL,
#endif
                       MKDEV(REALMAGIC_MAJOR, 3),
                       &card->dev->dev,
                       "%s%d",
                       REALMAGIC_DEVICE_NAME, 3);
}

static void udev_unregister_card(struct em84xx* card) {

    device_destroy(em84xx_class, MKDEV(REALMAGIC_MAJOR, 0));
    device_destroy(em84xx_class, MKDEV(REALMAGIC_MAJOR, 1));
    device_destroy(em84xx_class, MKDEV(REALMAGIC_MAJOR, 2));
    device_destroy(em84xx_class, MKDEV(REALMAGIC_MAJOR, 3));
}

struct registrar_s udev_registrar = {
    .register_driver    = &udev_register_driver,
    .unregister_driver  = &udev_unregister_driver,
    .register_card      = &udev_register_card,
    .unregister_card    = &udev_unregister_card,
};

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,2) */

struct registrar_s udev_registrar = {
    .register_driver    = NULL,
    .unregister_driver  = NULL,
    .register_card      = NULL,
    .unregister_card    = NULL,
};

#endif /*LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,2)*/
