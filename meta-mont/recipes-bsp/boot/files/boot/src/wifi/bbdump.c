#if 0
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <asm/mach-panther/panther.h>
#include <asm/mach-panther/tsi.h>
#include <asm/mach-panther/pmu.h>
#include <asm/mach-panther/reg.h>

#include <dragonite_main.h>
#else
#define DRAGONITE_BB_DUMP
#define printk printf

#include <mt_types.h>
#include <arch/chip.h>
#include <tsi.h>
#include <pmu.h>
#include <reg.h>
#endif

int printf(char *fmt, ...);
void udelay(unsigned int us);

#if defined(DRAGONITE_BB_DUMP)

#define PING_PONG_BUF

#define BB_DUMP_CTRL       (BASEBAND_REG_BASE + 0x0C0)
   #define BB_DUMP_DATA_SEL     0x0000001F
        #define DUMP_DATA_ADC         0x0
        #define DUMP_DATA_ADC_IQ      0xd
   #define BB_DUMP_TRIG_SEL     0x000F0000
        #define TRIG_DISABLE     0x0
        #define TRIG_IMMEDIATELY 0x1
        #define TRIG_FCS_PASS    0x2
        #define TRIG_FCS_FAIL    0x3
   #define BB_DUMP_EN           0x00000080

#define BB_DUMP_TAIL_TIME  (BASEBAND_REG_BASE + 0x0C4)   /* unit: 12.5ns */
#define BB_DUMP_CTRL2      (BASEBAND_REG_BASE + 0x0C8)
   #define BB_DUMP_CTRL2_BUF_OV 0x00010000
#define BB_DUMP_SIG_PTN    (BASEBAND_REG_BASE + 0x0CC)
#define BB_DUMP_MCH_PTN    (BASEBAND_REG_BASE + 0x0D0)
#define BB_DUMP_MCH_PTN2   (BASEBAND_REG_BASE + 0x0D4)
#define BB_DUMP_MCH_MASK   (BASEBAND_REG_BASE + 0x0D8)
#define BB_DUMP_MCH_MASK2  (BASEBAND_REG_BASE + 0x0DC)

#ifdef PING_PONG_BUF
#define DRAM_BUF_SIZE   (56  * 1024 * 1024)
#else
#define DRAM_BUF_SIZE   (16  * 1024 * 1024)
#endif
#define SRAM_BUF_SIZE   (16 * 1024)

unsigned int tsi_status;
unsigned int buf_overflow;
unsigned int dump_cur_addr;
void bb_dump(void)
{
   //printk("bb_adc_dump()\n");

   /* TSI source select 80Mhz BB clock */
   REG_UPDATE32(PMU_CLOCK_SELECT, TSI_BB_SEL, TSI_BB_SEL);

   /* set destination address and size */
#ifdef PING_PONG_BUF
   REG_WRITE32(ADC_DUMP_DST_BASE_ADDR_0, 0x00800000);       // address 8Mbytes
   REG_WRITE32(ADC_DUMP_DST_MEM_SIZE_0, (DRAM_BUF_SIZE / 8));

   REG_WRITE32(ADC_DUMP_DST_BASE_ADDR_1, 0x00800000);       // address 8Mbytes
   REG_WRITE32(ADC_DUMP_DST_MEM_SIZE_1, (DRAM_BUF_SIZE / 8));
#else
   REG_WRITE32(ADC_DUMP_DST_BASE_ADDR_0, 0x02000000);       // address 32Mbytes
   REG_WRITE32(ADC_DUMP_DST_MEM_SIZE_0, (DRAM_BUF_SIZE / 8));

   REG_WRITE32(ADC_DUMP_DST_BASE_ADDR_1, 0x02000000);       // address 32Mbytes
   REG_WRITE32(ADC_DUMP_DST_MEM_SIZE_1, (DRAM_BUF_SIZE / 8));
#endif

   /* set SRAM buffer address and size, max size is 16KB */
   REG_WRITE32(ADC_DUMP_BUF_START_ADDR, 0x10000000);        /* SRAM0 */
   REG_WRITE32(ADC_DUMP_BUF_SIZE, (SRAM_BUF_SIZE / 8));

   /* set SRAM buffer max burst count */
   // bit[23:16] = (SRAM buffer size /(tsi_burst_size+1)) - 1

   REG_UPDATE32(TSI_PKT_SIZE_RD_TH, (((SRAM_BUF_SIZE/TSI_DEFAULT_BURST_SIZE)-1)<<16), TSI_BUF_MAX_BURST_NUM);

   /* set TSI op mode */
#if defined(BIG_ENDIAN)
   REG_UPDATE32(TSI_ADCDUMP_CTRL, TSI_BYTE_SWAP | TSI_OP_MODE | TSI_TRANSFER_MODE,
                 TSI_BYTE_SWAP | TSI_OP_MODE | TSI_TRANSFER_MODE );
#else
   REG_UPDATE32(TSI_ADCDUMP_CTRL, TSI_OP_MODE | TSI_TRANSFER_MODE,
                 TSI_OP_MODE | TSI_TRANSFER_MODE );
#endif

   /* enable TSI */
   REG_UPDATE32(TSI_ADCDUMP_CTRL, TSI_ENABLE, TSI_ENABLE);

   /* configure BB */

   REG_WRITE32(BB_DUMP_TAIL_TIME, 800);
   REG_WRITE32(BB_DUMP_CTRL,(BB_DUMP_EN | (TRIG_FCS_PASS << 16) | DUMP_DATA_ADC_IQ));

   printk("bb_adc_dump() hw start\n");

#if 0
   /* wait memory dump dst change event */
   while (0==(REG_READ32(TSI_INTR_STATUS) & 0x00200000UL))
   {
      printk("INTR %08x\n", REG_READ32(TSI_INTR_STATUS));
   }
#endif

// schedule_timeout_interruptible(HZ);
#ifdef PING_PONG_BUF
   udelay(5000000);
#else
   udelay(3000000);
#endif

   tsi_status = (REG_READ32(TSI_STATUS) & (TSI_CORE_IDLE | TSI_AXI_MASTER_IDLE));
   buf_overflow = (REG_READ32(BB_DUMP_CTRL2) & BB_DUMP_CTRL2_BUF_OV);
   dump_cur_addr = REG_READ32(TSI_ADC_DUMP_DST_CURR_ADDR);
   printk("Check TSI status: %x\n", tsi_status);
   printk("Check BB buffer overflow: %x\n", buf_overflow);
   printk("TSI_ADC_DUMP_DST_CURR_ADDR: %x\n", dump_cur_addr);

   printk("bb_adc_dump() hw done\n");
   REG_WRITE32(BB_DUMP_CTRL, 0x0);
   //sim_memdump((unsigned char *)0xaf000000, DRAM_BUF_SIZE);
}

int is_dump_finish(void)
{
    if (tsi_status == (TSI_CORE_IDLE | TSI_AXI_MASTER_IDLE))
    {
        return 1;
    }

    return 0;
}

int is_bb_buffer_overflow(void)
{
    if (buf_overflow == BB_DUMP_CTRL2_BUF_OV)
    {
        return 1;
    }

    return 0;
}

unsigned int get_bbdump_cur_addr(void)
{
    return dump_cur_addr;
}

// special function for Barrios
void bb_dump_init_setting(void)
{
    /* TSI source select 80Mhz BB clock */
   REG_UPDATE32(PMU_CLOCK_SELECT, TSI_BB_SEL, TSI_BB_SEL);

   /* set destination address and size */
   REG_WRITE32(ADC_DUMP_DST_BASE_ADDR_0, 0x02000000);       // address 32Mbytes
   REG_WRITE32(ADC_DUMP_DST_MEM_SIZE_0, (DRAM_BUF_SIZE / 8));

   REG_WRITE32(ADC_DUMP_DST_BASE_ADDR_1, 0x02000000);       // address 32Mbytes
   REG_WRITE32(ADC_DUMP_DST_MEM_SIZE_1, (DRAM_BUF_SIZE / 8));

   /* set SRAM buffer address and size, max size is 16KB */
   REG_WRITE32(ADC_DUMP_BUF_START_ADDR, 0x10000000);        /* SRAM0 */
   REG_WRITE32(ADC_DUMP_BUF_SIZE, (SRAM_BUF_SIZE / 8));

   /* set SRAM buffer max burst count */
   // bit[23:16] = (SRAM buffer size /(tsi_burst_size+1)) - 1

   REG_UPDATE32(TSI_PKT_SIZE_RD_TH, (((SRAM_BUF_SIZE/TSI_DEFAULT_BURST_SIZE)-1)<<16), TSI_BUF_MAX_BURST_NUM);

   /* set TSI op mode */
#if defined(BIG_ENDIAN)
   REG_UPDATE32(TSI_ADCDUMP_CTRL, TSI_BYTE_SWAP | TSI_OP_MODE | TSI_TRANSFER_MODE,
                 TSI_BYTE_SWAP | TSI_OP_MODE | TSI_TRANSFER_MODE );
#else
   REG_UPDATE32(TSI_ADCDUMP_CTRL, TSI_OP_MODE | TSI_TRANSFER_MODE,
                 TSI_OP_MODE | TSI_TRANSFER_MODE );
#endif

   /* enable TSI */
   REG_UPDATE32(TSI_ADCDUMP_CTRL, TSI_ENABLE, TSI_ENABLE);

   /* configure BB */

   REG_WRITE32(BB_DUMP_TAIL_TIME, 800);
   REG_WRITE32(BB_DUMP_CTRL,((TRIG_FCS_PASS << 16) | DUMP_DATA_ADC_IQ));
}

// special function for Barrios
void bb_dump_start(void)
{
    int idx, wait_count;
    REG_UPDATE32(BB_DUMP_CTRL, BB_DUMP_EN, BB_DUMP_EN);

    printk("bb_adc_dump() hw start\n");

    // schedule_timeout_interruptible(HZ);
    for(idx=0;idx<10;idx++)
    {
        for(wait_count=0;wait_count<1000;wait_count++);
        tsi_status = (REG_READ32(TSI_STATUS) & (TSI_CORE_IDLE | TSI_AXI_MASTER_IDLE));
        if(tsi_status == (TSI_CORE_IDLE | TSI_AXI_MASTER_IDLE))
        {
            break;
        }
    }
    buf_overflow = (REG_READ32(BB_DUMP_CTRL2) & BB_DUMP_CTRL2_BUF_OV);
    dump_cur_addr = REG_READ32(TSI_ADC_DUMP_DST_CURR_ADDR);
    printk("Check TSI status: %x\n", tsi_status);
    printk("Check BB buffer overflow: %x\n", buf_overflow);
    printk("TSI_ADC_DUMP_DST_CURR_ADDR: %x\n", dump_cur_addr);

    printk("bb_adc_dump() hw done\n");
    REG_WRITE32(BB_DUMP_CTRL, 0x0);
}

#endif // DRAGONITE_BB_DUMP
