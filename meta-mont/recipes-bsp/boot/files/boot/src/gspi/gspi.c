#include <common.h>
#include <lib.h>
#include <mt_types.h>
#include <arch/chip.h>
#include "../sflash/include/flash_config.h"

#undef REG_READ32
#undef REG_WRITE32
#undef REG_UPDATE32

#undef REG_BASE
#define REG_BASE 0xBF000000UL

#define ADDRESS_4_DATA        0x00 //0xA4
#define TARGET_PHY_CONTROL_WORD_ADDR  ( ((u32) ADDRESS_4_DATA << 24) | 0xDB7F00 )
#define TARGET_PHY_MEM_ADDR   ( ((u32) ADDRESS_4_DATA << 24) | 0xDB7FD0 ) //0x09DB7FD

#define REG_READ32(x)   (*(volatile u32 *)(REG_BASE+(x)))
#define REG_WRITE32(x,val)  (*(volatile u32 *)(REG_BASE+(x)) = (u32)(val))
#define REG_UPDATE32(x,val,mask) do {\
    u32 newval; \
    newval = *(volatile u32 *) (REG_BASE+(x)); \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile u32*)(REG_BASE+(x)) = newval;    \
} while(0)

#define GPIO_SELECT_REG_BASE   0x00004A18

#define GPIO_SEL5_REG  0x4A70
#define SF_CS_SEL_MASK 0x00F00000

#define CMD_RDST   0x0F
#define CMD_SET    0x1F
#define CMD_WRITE  0x1B
#define CMD_BLKRD  0x0B
#define CMD_RDCACH 0x0D

static void gspi_gpio_pinmux_select(unsigned long pin, unsigned long sel)
{
    int reg_offset;
    int bit_offset;
    unsigned long reg_val;

    reg_offset = (pin / 8) * 4;
    bit_offset = (pin % 8) * 4;

    reg_val = REG_READ32(GPIO_SELECT_REG_BASE + reg_offset);

    reg_val = (reg_val & ~(0x0f << bit_offset)) | ((sel & 0x0f) << bit_offset);

    REG_WRITE32(GPIO_SELECT_REG_BASE + reg_offset, reg_val);
}

#define GCI0_STATUS  0x4900

#define GSPIS_CSR               0x9000
   #define GSPI_SLAVE_MODE      0x80000000
   #define GSPI_INT_MASK        0x40000000
   #define GSPI_4ADDR_MODE      0x00008000
   #define GSPI_INTR_STATUS     0x00002000
#define GSPIS_OP1               0x9004
#define GSPIS_OP2               0x9008

u32 gspi_ctrl_buf[0x1000 / 4];

/*!
 * function: gspi_send_cmd
 *  \brief serial flash read word
 *  \param cmd1_ionum:
 *  \param cmd1_ddr_mode:
 *  \param cmd1_length:
 *  \param cmd0_ionum:
 *  \param cmd0_ddr_mode:
 *  \param cmd0_length:
 *  \param data_ionum:
 *  \param data_ddr_mode:
 *  \param trans_cnt:
 *  \param trans_dir: 2:Rx, 1:Tx, 3:Rx/Tx
 *  \param dummy_length:
 *  \param channel:
 *  \return
 */
void gspi_send_cmd(cmd1_ionum, cmd1_ddr_mode, cmd1_length, cmd0_ionum,
                  cmd0_ddr_mode, cmd0_length, data_ionum, data_ddr_mode,
                  trans_cnt, trans_dir, dummy_length, channel)
{
    GSPIREG(SPI_TC) = trans_cnt;
    GSPIREG(SPI_CTRL) =
        (0x1 | (trans_dir << 1) | (dummy_length << 3) | (1 << (channel + 9)) |
         (data_ddr_mode << 13) | (data_ionum << 14) | (cmd0_length << 16) |
         (cmd0_ddr_mode << 21) | (cmd0_ionum << 22) | (cmd1_length << 24) |
         (cmd1_ddr_mode << 29) | (cmd1_ionum << 30));
}


/*!
 * function: gspi_write_cmd
 *  \brief serial flash write command
 *  \param write_cnt: count (unit: 4yte)
 *  \return
 */
void gspi_write_cmd(write_cnt)
{
    u32 i, j;

    for (i = 0; i < write_cnt; i = i + 1)
    {
        j = i % 4;
        GSPIREG(CMD_FIFO) = gspi_ctrl_buf[(i / 4)] >> ((3 - j) * 8);
    }
}

/*!
 * function: gspi_read_data
 *  \brief serial flash read word
 *  \param read_cnt: count (unit: 4yte)
 *  \param dma_sel: dma or not
 *  \return
 */
void gspi_read_data(u32 src, u32 read_cnt, u32 dma_sel, u32 aes_control)
{
    u32 read_counter, read_res_counter, read_fifo_cnt, ahb_read_data;
    u32 i;

    read_counter = 0;
    read_res_counter = read_cnt;
    while (read_counter != read_cnt)
    {
        read_fifo_cnt = 0;
        ahb_read_data = GSPIREG(STA);
        read_fifo_cnt = ahb_read_data & 0x3f;
        if (read_res_counter >= read_fifo_cnt)
        {
            for (i = 0; i < read_fifo_cnt; i = i + 1)
            {
                if (src != 0)
                {
                    *(volatile u32 *) (src + i * 4) = GSPIREG(DFIFO);
                }
                else
                {
                    gspi_ctrl_buf[read_counter + i] = GSPIREG(DFIFO);
                }
            }
            read_counter = read_counter + read_fifo_cnt;
            read_res_counter = read_res_counter - read_fifo_cnt;
        }
        else
        {
            for (i = 0; i < read_res_counter; i = i + 1)
            {
                if (src != 0)
                {
                    *(volatile u32 *) (src + i * 4) = GSPIREG(DFIFO);
                }
                else
                {
                    gspi_ctrl_buf[read_counter + i] = GSPIREG(DFIFO);
                }
            }
            read_counter = read_cnt;
            read_res_counter = 0;
        }
    }
}

/*!
 * function: gspi_write_data
 *  \brief serial flash write word
 *  \param write_cnt: count (unit: 4yte)
 *  \param dma_sel: dma or not
 *  \param index: start index
 *  \return
 */
void gspi_write_data(u32 write_cnt, u32 dma_sel, u32 index, u32 aes_control)
{
    u32 write_counter, write_res_counter, write_fifo_cnt, ahb_read_data;
    u32 i;

    {
        write_counter = 0;
        write_res_counter = write_cnt;
        while (write_counter != write_cnt)
        {
            ahb_read_data = GSPIREG(STA);
            write_fifo_cnt = 32 - ((ahb_read_data >> SPI_TCSHFT) & 0x3f);

            if (write_res_counter >= write_fifo_cnt)
            {
                for (i = 0; i < write_fifo_cnt; i = i + 1)
                {
                    GSPIREG(DFIFO) = gspi_ctrl_buf[write_counter + i + index];
                }
                write_counter = write_counter + write_fifo_cnt;
                write_res_counter = write_res_counter - write_fifo_cnt;
            }
            else
            {
                for (i = 0; i < write_res_counter; i = i + 1)
                {
                    GSPIREG(DFIFO) = gspi_ctrl_buf[write_counter + i + index];
                }
                write_counter = write_cnt;
                write_res_counter = 0;
            }
        }
    }
}

/*!
 * function: gspi_check_finish
 *  \brief wait SPI busy state gone
 *  \return
 */
void gspi_check_finish(void)
{
    u32 ahb_read_data;

    ahb_read_data = GSPIREG(STA);
    while (ahb_read_data & SPI_BUSY)
    {
        ahb_read_data = GSPIREG(STA);
    }
}

#define FEATURE_INTR     0x20
#define FEATURE_WRITE_EN 0x40
#define FEATURE_A4_BYTE  0x80

void gspi_set_address4(int raise_slave_intr)
{
   int channel = 0;
   u32 addr_cnt = 0, data_cnt = 2;

   gspi_ctrl_buf[0] = (CMD_SET << SF_CMD_S);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
   gspi_write_cmd(1 + addr_cnt);
   if(raise_slave_intr)
       gspi_ctrl_buf[0] = ((FEATURE_INTR | FEATURE_WRITE_EN)) | (ADDRESS_4_DATA << 8);
   else
       gspi_ctrl_buf[0] = ((FEATURE_WRITE_EN)) | (ADDRESS_4_DATA << 8);
   gspi_write_data(1, 0, 0, 0);
   gspi_check_finish();
}

void check_gspi_csr_status(void)
{
   int rc, channel = 0;
   u32 addr_cnt = 0, data_cnt = 2;

   gspi_ctrl_buf[0] = (CMD_RDST << SF_CMD_S);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
   gspi_write_cmd(1 + addr_cnt);
   gspi_read_data(0, 1, 0, 0);
   gspi_check_finish();

   rc = gspi_ctrl_buf[0] & 0xFFFF;
   printf("CSR status = 0x%x\n", rc);
}

void gspi_write_spi_u32(unsigned int addr, unsigned long value)
{
   int channel = 0;
   u32 addr_cnt = 3, data_cnt = 4;

   gspi_ctrl_buf[0] = (CMD_WRITE << SF_CMD_S) | (addr & 0xFFFFFF);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
   gspi_write_cmd(1 + addr_cnt);

   gspi_ctrl_buf[0] = value;

   gspi_write_data(data_cnt/4, 0, 0, 0);
   gspi_check_finish();
}

void gspi_write_spi(unsigned int addr)
{
   int channel = 0;
   int i;
   u32 addr_cnt = 3, data_cnt = 128;

   gspi_ctrl_buf[0] = (CMD_WRITE << SF_CMD_S) | (addr & 0xFFFFFF);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
   gspi_write_cmd(1 + addr_cnt);

   for (i = 0; i < data_cnt/4; i++)
   {
      gspi_ctrl_buf[i] = i*4 + ((i*4 + 1) << 8) + ((i*4 + 2) << 16) + ((i*4 + 3) << 24);
   }
   gspi_write_data(data_cnt/4, 0, 0, 0);
   gspi_check_finish();
}

void check_gspi_write_status(void)
{
   int rc, channel = 0;
   u32 addr_cnt = 0, data_cnt = 1;

   gspi_ctrl_buf[0] = (CMD_RDST << SF_CMD_S);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
   gspi_write_cmd(1 + addr_cnt);
   gspi_read_data(0, 1, 0, 0);
   gspi_check_finish();

   rc = gspi_ctrl_buf[0] & 0xFF;
   printf("write status = 0x%x\n", rc);
}

int check_gspi_read_status(void)
{
   int rc, channel = 0;
   u32 addr_cnt = 0, data_cnt = 1;

   gspi_ctrl_buf[0] = (CMD_RDST << SF_CMD_S);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
   gspi_write_cmd(1 + addr_cnt);
   gspi_read_data(0, 1, 0, 0);
   gspi_check_finish();

   rc = gspi_ctrl_buf[0];
   printf("read status = 0x%x\n", rc);

   return rc;
}

void block_read(unsigned int addr)
{
   int rc, channel = 0;
   u32 addr_cnt = 3, data_cnt = 0;

   gspi_ctrl_buf[0] = (CMD_BLKRD << SF_CMD_S) | (addr & 0xFFFFFF);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
   gspi_write_cmd(1 + addr_cnt);
   gspi_check_finish();

   while (1)
   {
      rc = check_gspi_read_status();

      if ((rc & 0x1000) != 0)
      {
         break;
      }
   }
}

static unsigned char data_counter = 0;
void read_cache_data(unsigned int len)
{
   unsigned int rc, channel = 0;
   u32 addr_cnt = 0, data_cnt = len;
   int i;
   unsigned int expect_data;

   gspi_ctrl_buf[0] = (CMD_RDCACH << SF_CMD_S);
   gspi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
   gspi_write_cmd(1 + addr_cnt);
   gspi_read_data(0, (data_cnt+3)/4, 0, 0);
   gspi_check_finish();

   for (i = 0; i < (data_cnt+3)/4; i++)
   {
      rc = gspi_ctrl_buf[i];
      //printf("GSPI read data = 0x%x, index = %d\n", rc, i);

      expect_data = (data_counter | ((data_counter+1) << 8) | ((data_counter+2) << 16) | ((data_counter+3) << 24));
      data_counter += 4;
      if(rc != expect_data)
      {
          printf("%d %08x %08x\n", i, rc, expect_data);
          printf("Failed.\n");
      }
   }
}

void check_intr_ctlr_status(void)
{
   int rc;

   rc = REG_READ32(GCI0_STATUS);
   if (((rc >> 23) & 0x1) == 0)
   {
      printf("ERROR GSPIS: No Interrupt Status\n");
   }
}

void clean_gspi_intr(void)
{
   int rc;

   rc = REG_READ32(GSPIS_CSR);
   REG_WRITE32(GSPIS_CSR, rc);

   rc = REG_READ32(GCI0_STATUS);
   if (((rc >> 23) & 0x1) != 0)
   {
      printf("ERROR GSPIS: Clear Interrupt Failed\n");
   }
}

void direct_mem_check()
{
   int i;
   unsigned char old_c = 0;
   unsigned char c;

   for (i = 0; i < 128; i++)
   {
      c = *(unsigned char *)((0xA0000000 | TARGET_PHY_MEM_ADDR) + i);
      if (c != old_c)
      {
         printf("got wrong data, read data = %x, old_c = %x\n", c, old_c);
         printf("Failed\n");
      }

      old_c++;
   }
}

static unsigned long clkdiv = 0x08;
void gspi_master_init(void)
{
    GSPIREG(CH0_BAUD) = ((SPI_INT_DLY << SPI_DLYSHFT) | clkdiv);
}

void gspi_slave_debug_init(void)
{
    REG_WRITE32(0x4A18, 0xBBBBBBBB);
    REG_WRITE32(0x4A1C, 0xBBBBBBBB);
    REG_UPDATE32(0x4A20, 0xB, 0xF);
    REG_WRITE32(0x4AAC, 0x80000000);
    REG_WRITE32(0x4AB0, 0x421);
}

#define GSPI_ROLE_MASTER 0
#define GSPI_ROLE_SLAVE  1
static unsigned int pinmux = 0;
int gspi_test(int role, int iterations)
{
   int rc;
   int i;

   // IO pad driving
   *(volatile unsigned long *)0xbf004a2c = 0xffffffffUL;
   *(volatile unsigned long *)0xbf004a30 = 0x00ffffffUL;

   if(pinmux == 1)
   {
       printf("Using GPIO17~20\n");

       gspi_gpio_pinmux_select(17, 0);
       gspi_gpio_pinmux_select(18, 0);
       gspi_gpio_pinmux_select(19, 0);
       gspi_gpio_pinmux_select(20, 0);

       gspi_gpio_pinmux_select(5, 0);
       gspi_gpio_pinmux_select(6, 0);
       gspi_gpio_pinmux_select(7, 0);
       gspi_gpio_pinmux_select(8, 0);
   }
   else if(pinmux == 0)
   {
       printf("Using GPIO5~8\n");

       gspi_gpio_pinmux_select(5, 5);
       gspi_gpio_pinmux_select(6, 5);
       gspi_gpio_pinmux_select(7, 5);
       gspi_gpio_pinmux_select(8, 5);

       gspi_gpio_pinmux_select(17, 3);
       gspi_gpio_pinmux_select(18, 3);
       gspi_gpio_pinmux_select(19, 3);
       gspi_gpio_pinmux_select(20, 3);
   }

   if(GSPI_ROLE_SLAVE==role)
   {
   #if 1
      // clear sf_cs_sel to default value
      REG_UPDATE32(GPIO_SEL5_REG, 0, SF_CS_SEL_MASK);
   #endif

      /* set to slave mode */
      REG_UPDATE32(GSPIS_CSR, GSPI_SLAVE_MODE, GSPI_SLAVE_MODE);

      /* enable interrupt */
      REG_UPDATE32(GSPIS_CSR, 0, GSPI_INT_MASK);

      //gspi_slave_debug_init();
      *(volatile unsigned long *) (0xA0000000|TARGET_PHY_CONTROL_WORD_ADDR) = 0;

      printf("GSPI slave mode turned on.\n");

      i = 1;
      while(1)
      {
          if(*(volatile unsigned long *) (0xA0000000|TARGET_PHY_CONTROL_WORD_ADDR))
          {
              *(volatile unsigned long *) (0xA0000000|TARGET_PHY_CONTROL_WORD_ADDR) = 0;
              i = 1;
          }
              
          if(REG_READ32(GSPIS_CSR) & GSPI_INTR_STATUS)
          {
              printf("RECV %d\n", i++);
    
              // check interrupt controller status
              check_intr_ctlr_status();
        
              // clean GSPIS interrupt
              clean_gspi_intr();
          }
      }
   }
   else if(GSPI_ROLE_MASTER==role)
   {
       gspi_master_init();
    #if 1
       // clear sf_cs_sel to default value
       REG_UPDATE32(GPIO_SEL5_REG, 0, SF_CS_SEL_MASK);
    #endif

    #if 1   // check OP1 and OP2 register default value
       rc = REG_READ32(GSPIS_OP1);
       printf("GSPIS_OP1 default value = 0x%x\n", rc);
       if (rc != 0xF1F1B0B)
       {
          printf("Have wrong default value of OP1 register\n");
       }
       rc = REG_READ32(GSPIS_OP2);
       printf("GSPIS_OP2 default value = 0x%x\n", rc);
       if (rc != 0xD)
       {
          printf("Have wrong default value of OP2 register\n");
       }
    #endif

       // set feature, a4
       gspi_set_address4(0);
       // check GSPI CSR status
       check_gspi_csr_status();

       gspi_write_spi_u32(TARGET_PHY_CONTROL_WORD_ADDR, 0x12345678);
       check_gspi_csr_status();

       for(i=1;i<=iterations;i++)
       {
              printf("Test %d\n", i);

              data_counter = 0;

              // write SPI
              gspi_write_spi(TARGET_PHY_MEM_ADDR);

              // check write status
              check_gspi_write_status();

              // block read, addr = 28'h9_db_7f_d0
              block_read(TARGET_PHY_MEM_ADDR);

              // read data from cache
              read_cache_data(48);

              // check read status(cache read)
              check_gspi_csr_status();

              // block read, addr = 28'h9_db_80_00
              block_read(TARGET_PHY_MEM_ADDR+48);

              // read data from cache
              read_cache_data(64);

              // check read status(cache read)
              check_gspi_csr_status();

              // block read, addr = 28'h9_db_80_40
              block_read(TARGET_PHY_MEM_ADDR+48+64);

              // read data from cache
              read_cache_data(16);

              // check read status(cache read)
              check_gspi_csr_status();

              // set feature, a4
              gspi_set_address4(1);
              // check GSPI CSR status
              check_gspi_csr_status();

              printf("Test %d done.\n", i);
       }
   }
   else
   {
       return ERR_PARM;
   }

   return ERR_OK;
}

int gspit_cmd(int argc, char *argv[])
{
    int iterations = 1;

    if(!strcmp(argv[0], "master"))
    {
        if(argc >= 2)
            iterations = atoi(argv[1]);

        return gspi_test(GSPI_ROLE_MASTER, iterations);
    }
    else if(!strcmp(argv[0], "slave"))
    {
        return gspi_test(GSPI_ROLE_SLAVE, iterations);
    }
    else if(!strcmp(argv[0], "pinmux"))
    {
        if(argc >= 2)
            pinmux = atoi(argv[1]);

        printf("SPI pinmux %d %s\n", pinmux
               , (pinmux==1) ?  "GPIO 17~20" : "GPIO 5~8");
        return ERR_OK;
    }
    else if(!strcmp(argv[0], "clkdiv"))
    {
        if(argc >= 2)
            clkdiv = atoi(argv[1]);

        printf("SPI master clock %dMhz, clkdiv %d\n", 120/clkdiv, clkdiv);
        return ERR_OK;
    }
    else
        return ERR_HELP;
}

cmdt cmdt_gspit __attribute__ ((section("cmdt"))) =
{
"gspit", gspit_cmd, "gspit [master|slave] [iterations]\n"
                    "gspit clkdiv [clock devider]\n"
                    "gspit pinmux [pinmux select]\n"
                    "   pinmux select 0:GPIO5~8 1:GPIO17~20"
};

