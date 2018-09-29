/*!
*   \file vbus_detection.c
*   \brief vbus detection driver.
*   \author Montage
*/
#ifdef CONFIG_VBUS_DETECTION
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#ifdef __ECOS
#include <os_api.h>             //include interrupt API
#include <cyg/hal/drv_api.h>    //header include interrupt function
#include <string.h>
#else
#include <common.h>
#include <arch/irq.h>
#include <arch/chip.h>
#endif
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define printd printf
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct vbus_detection_dev
{
    unsigned short irq;         /* GDMA IRQ number */
};
struct vbus_detection_dev vbus_detection_devx;
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void vbus_detection_intr_init(struct vbus_detection_dev *dev)
    __attribute__ ((section(".minit")));
void vbus_detection_init(void) __attribute__ ((section(".minit")));
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static void vbus_detection_irqhandler(void *param)
{
    struct vbus_detection_dev *dev = param;
    int r = (USBREG(OTGSC) & AVV);

    /* mask interrupt */
    disable_irq(dev->irq);

    /* clear latch interrupt */
    USBREG(OTGSC) |= AVVIS;

    printd("Vbus %s Detection!\n", r ? "Rising" : "Falling");

    /* on/off USB module */
    if (r)
        USBREG(USB_A0_REG) |= USB_DOWN;
    else
        USBREG(USB_A0_REG) &= ~USB_DOWN;

    /* unmask interrupt */
    enable_irq(dev->irq);
}

void vbus_detection_intr_init(struct vbus_detection_dev *dev)
{
    dev->irq = IRQ_UDC;

#ifdef __ECOS
    /* mask interrupt */
    cyg_drv_interrupt_mask(dev->irq);

    /* Interrupt and Interrupt Handler Configuration */
    //cyg_drv_interrupt_create(dev->irq, 0, (CYG_ADDRWORD)dev,
    //  gdma_isr, gdma_dsr, &dev->intr_handle, &dev->intr_obj);

    /* hook handler */
    cyg_drv_interrupt_attach(dev->intr_handle);

    /* IRQ number and trigger Configuration */
    cyg_drv_interrupt_configure(dev->irq, 1, 1);

    /* umask interrupt and enable interrupt work */
    cyg_drv_interrupt_unmask(dev->irq);
#else
    request_irq(dev->irq, &vbus_detection_irqhandler, (void *) dev);
#endif
}

void vbus_detection_init(void)
{
    struct vbus_detection_dev *dev = &vbus_detection_devx;

    /* initialize registers */
    USBREG(PHY_DIG_CTRL) |= VBUS_DETECTION;
    // VBus Valid Interrupt Enable
    USBREG(OTGSC) |= AVVIE;

    /* initialize interrupt settting */
    vbus_detection_intr_init(dev);
}
#endif
