/*****************************************************************************/

/*
 *     devfs.c -- NetStream2000 EM8400 (quasar) driver
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


#include "devfs.h"

#ifdef CONFIG_DEVFS_FS

//#include <linux/config.h>
#include <linux/devfs_fs_kernel.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,70)
devfs_handle_t em84xx_handle[4];
#endif

extern struct file_operations em8300_fops;

static void devfs_register_card(struct em84xx *card)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,70)
    char devname[64];
    sprintf(devname, "%s%d", REALMAGIC_DEVICE_NAME, 0);
    em8300_handle[0] = devfs_register(NULL, devname, DEVFS_FL_DEFAULT, REALMAGIC_MAJOR,
                            0, S_IFCHR | S_IRUGO | S_IWUGO, &em8300_fops, NULL);

    sprintf(devname, "%s%d", REALMAGIC_DEVICE_NAME, 1);
    em8300_handle[0] = devfs_register(NULL, devname, DEVFS_FL_DEFAULT, REALMAGIC_MAJOR,
                            1, S_IFCHR | S_IRUGO | S_IWUGO, &em8300_fops, NULL);

    sprintf(devname, "%s%d", REALMAGIC_DEVICE_NAME, 2);
    em8300_handle[0] = devfs_register(NULL, devname, DEVFS_FL_DEFAULT, REALMAGIC_MAJOR,
                            2, S_IFCHR | S_IRUGO | S_IWUGO, &em8300_fops, NULL);

    sprintf(devname, "%s%d", REALMAGIC_DEVICE_NAME, 3);
    em8300_handle[0] = devfs_register(NULL, devname, DEVFS_FL_DEFAULT, REALMAGIC_MAJOR,
                            3, S_IFCHR | S_IRUGO | S_IWUGO, &em8300_fops, NULL);
#else
    devfs_mk_cdev(MKDEV(REALMAGIC_MAJOR, 0),
              S_IFCHR | S_IRUGO | S_IWUGO,
              "%s%d", REALMAGIC_DEVICE_NAME, 0);

    devfs_mk_cdev(MKDEV(REALMAGIC_MAJOR, 1),
              S_IFCHR | S_IRUGO | S_IWUGO,
              "%s%d", REALMAGIC_DEVICE_NAME, 1);

    devfs_mk_cdev(MKDEV(REALMAGIC_MAJOR, 2),
              S_IFCHR | S_IRUGO | S_IWUGO,
              "%s%d", REALMAGIC_DEVICE_NAME, 2);

    devfs_mk_cdev(MKDEV(REALMAGIC_MAJOR, 3),
              S_IFCHR | S_IRUGO | S_IWUGO,
              "%s%d", REALMAGIC_DEVICE_NAME, 3);


#endif
}

static void devfs_unregister_card(struct em84xx *card)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,69)
    devfs_unregister(em8300_handle[0]);
    devfs_unregister(em8300_handle[1]);
    devfs_unregister(em8300_handle[2]);
    devfs_unregister(em8300_handle[3]);
#else
    devfs_remove("%s%d", REALMAGIC_DEVICE_NAME, 0);
    devfs_remove("%s%d", REALMAGIC_DEVICE_NAME, 1);
    devfs_remove("%s%d", REALMAGIC_DEVICE_NAME, 2);
    devfs_remove("%s%d", REALMAGIC_DEVICE_NAME, 3);
#endif
}

struct registrar_s devfs_registrar = {
    .register_driver    = NULL,
    .unregister_driver  = NULL,
    .register_card      = &devfs_register_card,
    .unregister_card    = &devfs_unregister_card,
};

#else

struct registrar_s devfs_registrar = {
    .register_driver    = NULL,
    .unregister_driver  = NULL,
    .register_card      = NULL,
    .unregister_card    = NULL,
};

#endif /*CONFIG_DEVFS_FS*/
