
#include <common.h>
#include <lib.h>
#include <netprot.h>
#include <mt_types.h>

#include <arch/chip.h>

#define MSG printf
void gpio_driver_show_info(void)
{
    MSG("\n");
    MSG("Valid    id [0 ~ 47]\n");
    MSG("Valid value [0/default: 4mA, 1: 8mA, 2: 12mA, 3: 16mA]\n");
    MSG("=== Specific gpio id ===\n");
    MSG("PAD_UFT[45]\nPAD_UFR[46]\n");
    //MSG(" SPI_CK[51],  SPI_CS[52], SPI_SDIO[53], SPI_SD1[54]\n");
    MSG("Reserved[25~32, 47]\n\n");
}

extern void pmu_set_gpio_driving_strength(int *gpio_ids, unsigned long *gpio_funcs);
void gpio_driver_strength_apply(void)
{
    int len;
    int i;
    char str_driving[256];
    char *p;
    int pin_no, func_sel;

    int gpio_ids[] = { -1, -1 };
    unsigned long gpio_funcs[] = { -1, -1 };

    len = strlen(bootvars.gpio_driving);
    if(len==0)
        return;

    strcpy(str_driving, bootvars.gpio_driving);

    p = str_driving;
    for(i=0;i<=len;i++)
    {
        if((str_driving[i]==',')||(str_driving[i]=='\0'))
        {
            str_driving[i] = '\0';
            if(2==sscanf(p, "%d=%d", &pin_no, &func_sel))
            {
                gpio_ids[0] = pin_no;
                gpio_funcs[0] = func_sel;
                pmu_set_gpio_driving_strength(gpio_ids, gpio_funcs);
            }
            p = &str_driving[i+1];
        }
    }
}

int gpio_driving_cmd(int argc, char *argv[])
{
    gpio_driver_show_info();
    return ERR_OK;
}


cmdt cmdt_gpio_driver_strength __attribute__ ((section("cmdt"))) =
{
"gpio_driving", gpio_driving_cmd, "show driving information"};

#define _GPIO_SETTING_INPUT       2
#define _GPIO_SETTING_OUTPUT_HIGH 1
#define _GPIO_SETTING_OUTPUT_LOW  0
void gpio_setting_apply(void)
{
    int len;
    int i;
    char str_gpio_setting[256];
    char *p;
    int gpio, gpio_setting;

    len = strlen(bootvars.gpio_setting);
    if(len==0)
        return;

    strcpy(str_gpio_setting, bootvars.gpio_setting);

    p = str_gpio_setting;
    for(i=0;i<=len;i++)
    {
        if((str_gpio_setting[i]==',')||(str_gpio_setting[i]=='\0'))
        {
            str_gpio_setting[i] = '\0';
            if(2==sscanf(p, "%d=%d", &gpio, &gpio_setting))
            {
                //printf("!!!! GPIO %d , setting %d\n", gpio, gpio_setting);
                PMUREG_UPDATE32(GPIO_FUNC_EN, (0x01 << gpio), (0x01 << gpio));
                if(gpio_setting==_GPIO_SETTING_INPUT)
                {
                    // input mode
                    PMUREG_UPDATE32(GPIO_OEN, (0x01 << gpio), (0x01 << gpio));
                }
                else if(gpio_setting==_GPIO_SETTING_OUTPUT_HIGH)
                {
                    // output 1
                    PMUREG_UPDATE32(GPIO_OEN, 0, (0x01 << gpio));
                    PMUREG_WRITE32(GPIO_ODS, (0x01 << gpio));
                }
                else if(gpio_setting==_GPIO_SETTING_OUTPUT_LOW)
                {
                    // output 0
                    PMUREG_UPDATE32(GPIO_OEN, 0, (0x01 << gpio));
                    PMUREG_WRITE32(GPIO_ODC, (0x01 << gpio));
                }
            }
            p = &str_gpio_setting[i+1];
        }
    }
}

