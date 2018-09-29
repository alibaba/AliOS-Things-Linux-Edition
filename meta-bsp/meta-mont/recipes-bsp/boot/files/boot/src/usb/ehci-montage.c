/*!
*   \file ehci-montage.c
*   \brief USB EHCI initialization for Montage USB
*   \author
*/
/*
 * (C) Copyright 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 *
 * Author: Michael Trimarchi <trimarchimichael@yahoo.it>
 * This code is based on ehci freescale driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifdef CONFIG_USB

#include <arch/chip.h>
#include <common.h>
#include <usb.h>

#include <ehci.h>
#include <ehci-core.h>

#define EHCI_PS_PFSC (1<<24)
int usb_select_port = 0;
int usb_force_fs = 0;
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(void)
{
    if(usb_select_port == 0) {
        hccr = (struct ehci_hccr *) (USB_BASE + 0x20);
        hcor = (struct ehci_hcor *) (USB_BASE + 0x40);

        ehci_writel(USB_BASE + 0x300, ehci_readl(USB_BASE + 0x300) | (0x1 << 6)); //adjust analog CDR sample cycle
    }
    else {
        hccr = (struct ehci_hccr *) (USB_OTG_BASE + 0x20);
        hcor = (struct ehci_hcor *) (USB_OTG_BASE + 0x40);

        ehci_writel(USB_OTG_BASE + 0x300, ehci_readl(USB_OTG_BASE + 0x300) | (0x1 << 6)); //adjust analog CDR sample cycle
    }
    printf("Montage USB init port %d\n", usb_select_port);

    ehci_writel(&hcor->or_usbmode, 3);  //OTG host mode
    ehci_writel(&hcor->burstsize, 0x1010);      //burst size 128 byte
    ehci_writel(&hcor->txfill, 0x20000);        //tx threshold is 4 burst
    if(usb_force_fs)
        ehci_writel(&hcor->or_portsc[0], EHCI_PS_PFSC);     //force to full speed

    return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
    if(usb_select_port == 0) {
        *(unsigned int *) (USB_BASE + 0x84) = 0x184;        //8 Port Reset/7 Port_Suspend/2 Port_enable
        *(unsigned int *) (USB_BASE + 0x40) = 0x0;  //host controller stop
    }
    else {
        *(unsigned int *) (USB_OTG_BASE + 0x84) = 0x184;        //8 Port Reset/7 Port_Suspend/2 Port_enable
        *(unsigned int *) (USB_OTG_BASE + 0x40) = 0x0;  //host controller stop
    }
    return 0;
}
#endif                          //CONFIG_USB
