#include <common.h>
#include <lib.h>
#include <netprot.h>

#define MSG printf
void pinmux_show_table(void)
{
    MSG("SEL DEF      0        1        2     3    4        5 6        7         8\n");
    MSG("  0   1   GPIO   I2C_SD     LED0            UART2_TX    TS_CLK           \n");
    MSG("  1   1   GPIO  I2C_SCL     LED1 SPDIF      UART2_RX    TS_VLD           \n");
    MSG("  2   0   GPIO   I2C_SD     LED2       PWM0 I2S_MCLK   TS_SYNC           \n");
    MSG("  3   0   GPIO  I2C_SCL     LED1       PWM1 I2S_BCLK    TS_ERR           \n");
    MSG("  4   0   GPIO              LED0 SPDIF PWM2  I2S_LRC     TS_D0           \n");
    MSG("  5   9   GPIO  PA_PAON     LED0 LCDDC PWM4   SPI_DI             RMII_CLK\n");
    MSG("  6   9   GPIO  PA_TXON     LED1       PWM3   SPI_DO     TS_D5  RMII_TXEN\n");
    MSG("  7   9   GPIO  PA_RXON     LED2       PWM2   SPI_CS     TS_D6  RMII_TXD0\n");
    MSG("  8   9   GPIO                         PWM1   SPI_CK     TS_D7  RMII_TXD1\n");
    MSG("  9   0   GPIO    EPAON                PWM0              TS_D1 RMII_CRSDV\n");
    MSG(" 10   0   GPIO   I2C_SD  PA_PAON            I2S_D_TX     TS_D2  RMII_RXD0\n");
    MSG(" 11   0   GPIO  I2C_SCL  PA_TXON            I2S_D_RX     TS_D3  RMII_RXD1\n");
    MSG(" 12   0   GPIO      LNA  PA_RXON                         TS_D4   RMII_MDC\n");
    MSG(" 13   0   GPIO                   SPDIF                          RMII_MDIO\n");
    MSG(" 14   0   GPIO  PA_PAON          SPDIF      I2S_MCLK     TS_D5           \n");
    MSG(" 15   0   GPIO  PA_TXON UART2_TX            I2S_BCLK     TS_D6           \n");
    MSG(" 16   0   GPIO  PA_RXON UART2_RX             I2S_LRC     TS_D7           \n");
    MSG(" 17   0 SPI_DI           I2C_SD  GPIO                                    \n");
    MSG(" 18   0 SPI_DO          I2C_SCL  GPIO                                    \n");
    MSG(" 19   0 SPI_CS         UART1_TX  GPIO                                    \n");
    MSG(" 20   0 SPI_CK         UART1_RX  GPIO                                    \n");
    MSG(" 21   1   GPIO UART1_TX                    I2S_D_TX                      \n");
    MSG(" 22   1   GPIO UART1_RX         SPDIF      I2S_D_RX                      \n");
    MSG(" 23   0 SF_SD2 UART3_TX  I2C_SD  GPIO                                    \n");
    MSG(" 24   0 SF_SD3 UART3_RX I2C_SCL  GPIO                                    \n");
    MSG(" 25   0 SD_SD0 UART2_TX                                                  \n");
    MSG(" 26   0 SD_SD1 UART2_RX                                                  \n");
    MSG(" 27   0 SD_SD2 UART3_TX                                                  \n");
    MSG(" 28   0 SD_SD3 UART3_RX                                                  \n");
    MSG(" 29   0 SD_CMD   I2C_SD                                                  \n");
    MSG(" 30   0  SD_WP  I2C_SCL                                                  \n");
    MSG(" 31   0  SD_CD     GPIO                                                  \n");
    MSG(" 32   0 SD_CLK     GPIO                                                  \n");
    MSG("SEL DEF      0        1        2     3    4        5 6        7         8\n");
}

extern void pmu_set_gpio_function(int *gpio_ids, unsigned long *gpio_funcs);
void pinmux_apply(void)
{
    int len;
    int i;
    char str_pinmux[256];
    char *p;
    int pin_no, func_sel;

    int gpio_ids[] = { -1, -1 };
    unsigned long gpio_funcs[] = { -1, -1 };

    len = strlen(bootvars.pinmux);
    if(len==0)
        return;

    strcpy(str_pinmux, bootvars.pinmux);

    p = str_pinmux;
    for(i=0;i<=len;i++)
    {
        if((str_pinmux[i]==',')||(str_pinmux[i]=='\0'))
        {
            str_pinmux[i] = '\0';
            if(2==sscanf(p, "%d=%d", &pin_no, &func_sel))
            {
                gpio_ids[0] = pin_no;
                gpio_funcs[0] = func_sel;
                pmu_set_gpio_function(gpio_ids, gpio_funcs);
            }
            p = &str_pinmux[i+1];
        }
    }
}

int pinmux_cmd(int argc, char *argv[])
{
    pinmux_show_table();
    return ERR_OK;
}


cmdt cmdt_pinmux __attribute__ ((section("cmdt"))) =
{
"pinmux", pinmux_cmd, "show pinmux table"};


