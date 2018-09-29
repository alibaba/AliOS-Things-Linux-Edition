/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cmd.c
*   \brief  Command  handler
*   \author Montage
*/

#include <arch/chip.h>
#include <arch/irq.h>
#include <common.h>
#include <netprot.h>
#include <image.h>
#include <ubimage.h>
#include <md5.h>
#include <lib.h>
#include <dhcpd.h>
#include <otp.h>
#include <cm_mac.h>
#include <pmu.h>
#include "sflash/include/flash_config.h"
#include "sflash/include/flash_api.h"

extern void start_boot_failure_detection(void);
extern int is_boot2(void);
extern void burn_linux_from_xmodem(void);

extern char ih_magic[];
extern bootvar bootvar_default;
#ifdef CONFIG_GDMA
extern void gdma_finish(void);
#endif
unsigned long buf_address;
unsigned long byte_count;
char *boot_mode_[] = { "cmd", "flash", "tftp", "boot3", "buf", "recovery", "mptool", "second"," " };

#if defined(CONFIG_CMD_FLASH) && defined(CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE)
char downloaded = 0;
#endif

/*!
 * function:
 *
 *  \brief
 *  \param mode
 *  \return
 */

char *boot_mode_str(int mode)
{
    return boot_mode_[3 & mode];
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
extern u32 is_enable_secure_boot;
extern int stop_boot_process(void);
int cmd_go(int argc, char *argv[])
{
    void (*go) (void);
    unsigned int flags;
    unsigned long load_loc = bootvars.load_addr;
    int rc;
    struct img_head *ih;
    int err = ERR_OK;
    unsigned int img_type = UNKNOWN_IMAGE_TYPE;

    if (argc > 0)
    {
        if (!strcmp(argv[0], boot_mode_[BOOT_MODE_FLASH]))    /* flash */
        {
            img_type = load_app_image((u32 *)&ih, bootvars.ci_offset);

            if (img_type == UNKNOWN_IMAGE_TYPE)
            {
                goto fail;
            }
            dbg_log(LOG_VERBOSE, "ih = 0x%x\n", ih);
            load_loc = (unsigned int) ih;
            goto chk_header;
        }
#ifdef CONFIG_CMD_TFTP
        else if (!strcmp(argv[0], boot_mode_[BOOT_MODE_TFTP]))       /* tftp */
        {
            eth_probe();
            cheetah_phy_up(1, 0);
            if (net_tftp(0, load_loc, 0, bootvars.file))
                goto chk_header;
        }
#endif                          //CONFIG_CMD_TFTP
        else if ((!strcmp(argv[0], boot_mode_[BOOT_MODE_BOOT3]))&&(!is_enable_secure_boot))
        {
            printf("go boot3, load_loc = 0x%x\n", load_loc);
            goto chk_header;
        }
        else if ((!strcmp(argv[0], boot_mode_[BOOT_MODE_BUF]))&&(!is_enable_secure_boot))
        {
            printf("go buf, load_loc = 0x%x\n", load_loc);
            goto chk_header;
        }
        else if (!strcmp(argv[0], boot_mode_[BOOT_MODE_RECOVERY]) && is_boot2() && bootvars.recovery == 1)
        {
            img_type = load_app_image((u32 *)&ih, bootvars.recovery_offset);

            if (img_type == UNKNOWN_IMAGE_TYPE)
            {
                err = ERR_GO_RECOVERY;
                goto fail;
            }
            dbg_log(LOG_VERBOSE, "ih = 0x%x\n", ih);
            load_loc = (unsigned long) ih;
            memcpy((void *)BOOT_CMDLINE_DRAM_ADDR, boot_mode_[BOOT_MODE_RECOVERY], COMMAND_LINE_SIZE);
            goto chk_header;
        }
        else if (!strcmp(argv[0], boot_mode_[BOOT_MODE_MPTOOL]) && is_boot2())
        {
            img_type = load_app_image((u32 *)&ih, bootvars.recovery_offset);

            if (img_type == UNKNOWN_IMAGE_TYPE)
            {
                err = ERR_GO_RECOVERY;
                goto fail;
            }
            dbg_log(LOG_VERBOSE, "ih = 0x%x\n", ih);
            load_loc = (unsigned long) ih;
            memcpy((void *)BOOT_CMDLINE_DRAM_ADDR, boot_mode_[BOOT_MODE_MPTOOL], COMMAND_LINE_SIZE);
            goto chk_header;
        }
        else if (!strcmp(argv[0], boot_mode_[BOOT_MODE_SECOND]) && is_boot2() && bootvars.recovery == 2)
        {
            img_type = load_app_image((u32 *)&ih, bootvars.second_img_offset);

            if(img_type == UNKNOWN_IMAGE_TYPE)
            {
                err = ERR_GO_SECOND;
                goto fail;
            }
            dbg_log(LOG_VERBOSE, "ih = 0x%x\n", ih);
            load_loc = (unsigned long) ih;
            goto chk_header;
        }

        goto help;
    }
    else
    {
        // if only type command with "g"/"go"
        img_type = load_app_image((u32 *)&ih, bootvars.ci_offset);
        if (img_type == UNKNOWN_IMAGE_TYPE)
        {
            goto fail;
        }
        printf("ih = 0x%x\n", ih);
        load_loc = (unsigned long) ih;
    }
chk_header:
    rc = load_image((char *)load_loc, &load_loc);
    if (rc == -2)
    {
        printf("User stop !\n\n");
        goto fail;
    }
    if (argc > 0 && !strcmp(argv[0], boot_mode_[BOOT_MODE_FLASH]))    /* flash */
    {
        if (rc <= 0)
        {
            printf("Failed to load image\n");
            err = ERR_PARM;
            goto fail;
        }
    }
    else
    {
        if (0 > rc)             // not an image, then going to this address directly
        {
            printf("Invalid image\n");
            err = ERR_FILE;
            goto fail;
        }
        else if (rc == 0)       // can't decompress
        {
            printf("Corrupted image\n");
            err = ERR_FILE;
            goto fail;
        }
    }
    if (vaddr_check(load_loc))
    {
        err = ERR_ADDR;
        goto fail;
    }

    if(stop_boot_process())
    {
        err = ERR_TIMEOUT;
        goto fail;
    }

    dbg_log(LOG_VERBOSE, "Go 0x%08x!\n", load_loc);
    go = (void *) (load_loc);

    flags = irq_save();
    /* disable or finish module would re-initialize in app */
    //eth_reset();
#ifdef CONFIG_GDMA
    gdma_finish();
#endif
    reset_devices();
    dcache_flush();
    icache_inv();

	if (!strcmp(argv[0], boot_mode_[BOOT_MODE_SECOND]) && is_boot2() && bootvars.recovery == 2)
	{
		//skip boot failure detection for second image
	}
	else
	{
        start_boot_failure_detection();
	}

    if((img_type==CI_IMAGE_TYPE)||(img_type==UBOOT_IMAGE_TYPE))
        powercfg_apply();

    go();
    irq_restore(flags);
    err = ERR_OK;

  help:
    return ERR_HELP;
  fail:
    // No need this now
//      #ifdef CONFIG_OVERCLOCKING_FOR_APP
//          cheetah_cpuclk_restore(flag);
//      #endif

    if((ERR_GO_RECOVERY != err)&&(ERR_TIMEOUT != err))
        start_boot_failure_detection();

    memset((void *)BOOT_CMDLINE_DRAM_ADDR, 0, COMMAND_LINE_SIZE);
    return err;
}

cmdt cmdt_go __attribute__ ((section("cmdt"))) =
{
"go", cmd_go, "go <addr> ;go to <addr>\n"};

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int cmd_help(int argc, char *argv[])
{
    int i;
    cmdt *cp;

    if (argc == 0)
    {
        printf("Commands:\n\r");
        for (i = 0, cp = &cmd_table[0]; i < cmdt_sz; i++, cp++)
        {
            printf("%-10s", cp->cmd);
            if ((i % 6) == 5)
                printf("\n");
        }
        printf("\n");
        return ERR_OK;
    }
    else
    {
        for (i = 0, cp = &cmd_table[0]; i < cmdt_sz; i++, cp++)
        {
            if (!strcmp(cp->cmd, argv[0]))
            {
                printf("%s \n", cp->msg);
                return ERR_OK;
            }
        }
    }
    return ERR_PARM;
}

cmdt cmdt_0help __attribute__ ((section("cmdt"))) =
{
"?", cmd_help, "? cmd"};

int cmd_ver(int argc, char *argv[])
{
    printf("%s\n", BOOT_REVNUM);
    return ERR_OK;
}

cmdt cmdt_ver __attribute__ ((section("cmdt"))) =
{
"ver", cmd_ver, "ver ;show boot-loader version"};

#ifdef  CONFIG_CMD_FLASH
/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
enum
{
    OP_ERASE = (1 << 0),
    OP_PROGRAM = (1 << 1),
    OP_VERIFY = (1 << 2),
    OP_READ = (1 << 3),
    OP_CDBSAVE = (1 << 4),
    OP_REBOOT = (1 << 5),
    OP_LINFO = (1 << 6),
    OP_LOCK = (1 << 7),
    OP_UNLOCK = (1 << 8),
    OP_FIND_BAD_BLOCK = (1 << 9),
    OP_MARK_BAD_BLOCK_RANGE = (1 << 10),
    OP_MARK_GOOD_BLOCK_RANGE = (1 << 11),
    OP_MARK_BAD_BLOCK = (1 << 12),
    OP_MARK_GOOD_BLOCK = (1 << 13),
    OP_WRITE_KEY = (1 << 14),
    OP_START_FIRMWARE = (1 << 15),
    OP_FULL_ERASE = (1 << 16),
};

extern void sf_flush_cache_all(void);
int flash_cmd(int argc, char *argv[])
{
    unsigned int src, dst, len;
    int rc;
    int force_write = 0;
    int opcode;
    int option = argv[-1][1];
    unsigned int ci_magic = cpu_to_be32(CI_MAGIC);
    unsigned int uboot_magic = cpu_to_be32(IH_UBMAGIC);

    dst = bootvars.ci_offset;

    src = bootvars.load_addr;
    // default with flash erase and program
    opcode = (OP_ERASE | OP_PROGRAM);
    // byte_count initial from cmd_tftp
    len = byte_count;

    // auto detect image type
    if (option == 0)
    {
        if (len==0)
        {
            printf("No image downloaded yet!\n");
            goto done;
        }

        if ((rc=is_boot_image(src)))
        {
            printf("Auto-detect: bootcode(%s)\n", (rc==BOOT_IMAGE_BE) ? "BE" : "LE");
            option = '1';
        }
        else if(is_secure_boot_image(src, len))
        {
            printf("Auto-detect: secure bootcode\n");
            option = '1';
        }
        else
        {
            printf("Auto-detect: firmware\n");
            option = 'a';
        }
    }

    switch (option)
    {
        case 'v':              // verify
            opcode |= OP_VERIFY;
            break;
        case 'e':              // erase
            opcode = OP_ERASE;
            break;
        case 'w':              // program
            opcode = OP_PROGRAM;
            break;
        case 'r':              // read
            opcode = OP_READ;
            break;
        case 'a':              // program app
            // check whether image is valid or not
            if (!memcmp((void *)&ci_magic, (void *)BOOT_BURN_BASE, 4))
            {
				printf("Don't support ci header in this boot!!!\n");
				goto fail;
            }
            else if (!memcmp((void *)&uboot_magic, (void *)BOOT_BURN_BASE, 4))
            {
                // Uboot-Image header
                rc = verify_ubimage(BOOT_BURN_BASE + sizeof(image_header_t));
                if (rc != 2)
                {
                    printf("Invalid UBOOT Image !!\n");
                    goto fail;
                }
            }

            if (argv[-1][2] == 'g')     // r means reboot
            {
                opcode |= (OP_VERIFY | OP_REBOOT);
            }

            if (argv[-1][2] == 's')     // s means start firmware
            {
                opcode |= OP_START_FIRMWARE;
            }

			//always erase flash
            flash_erase_appdata();
            opcode &= (~OP_ERASE);  // if have been erased app data, no need to erase again
            break;
        case 'b':              // program bootloader
            dst = 0;
            opcode |= OP_CDBSAVE;
            break;
        case 'c':              // erase cdb
            erase_setting_block();
            break;
#ifdef CONFIG_UART
        case 'f':
            burn_linux_from_xmodem();
            goto done;
#endif
        case '1':              // program boot.img into flash
            opcode = OP_ERASE | OP_PROGRAM | OP_WRITE_KEY;
            //len = BOOT_IMG_SIZE;
            dst = 0;
            break;
        case 'i':               // full flash erase & program flash image
            opcode = OP_FULL_ERASE | OP_PROGRAM;
            dst = 0;
            break;
#ifdef BAD_BLOCK_MANAGE         // function about bad block management
        case '3':
            opcode = OP_FIND_BAD_BLOCK;
            break;
        case '4':
            opcode = OP_MARK_BAD_BLOCK_RANGE;
            break;
        case '5':
            opcode = OP_MARK_GOOD_BLOCK_RANGE;
            break;
        case '6':
            opcode = OP_MARK_BAD_BLOCK;
            break;
        case '7':
            opcode = OP_MARK_GOOD_BLOCK;
            break;
#endif
#ifdef CONFIG_FLASH_SW_LOCK
        case 'n':              // lock information
            opcode = OP_LINFO;
            break;
        case 'l':              // lock
            opcode = OP_LOCK;
            break;
        case 'u':              // unlock
            opcode = OP_UNLOCK;
            break;
#endif
        case 'k':   // for recovery
            //if(!is_boot2() || bootvars.recovery == 0)
            if(bootvars.recovery_offset == 0)
                goto err1;
            opcode = OP_ERASE | OP_PROGRAM;
            dst = bootvars.recovery_offset;
            len = BOOT_RECOVERY_SIZE;
            break;
        case 't':
            if(bootvars.second_img_offset == 0)
            {
                goto err1;
            }
            opcode = OP_ERASE | OP_PROGRAM;
            dst = bootvars.second_img_offset;
            len = byte_count;
            break;
        default:
            break;
    }

#ifdef BAD_BLOCK_MANAGE         // function about bad block management
    if (argc > 0 && sscanf(argv[0], "%d", &dst) != 1)
    {
        goto err1;
    }
    if (argc > 1 && sscanf(argv[1], "%d", &len) != 1)
    {
        goto err1;
    }

    if (opcode & OP_FIND_BAD_BLOCK)
    {
        find_all_bad_block();
        goto done;
    }

    if (opcode & OP_MARK_BAD_BLOCK_RANGE)
    {
        mark_bad_block_by_interval(dst, len);
        goto done;
    }

    if (opcode & OP_MARK_GOOD_BLOCK_RANGE)
    {
        mark_good_block_by_interval(dst, len);
        goto done;
    }

    // if only have one argument, dst means block_index,
    if (opcode & OP_MARK_BAD_BLOCK)
    {
        mark_bad_block(dst);
        goto done;
    }

    if (opcode & OP_MARK_GOOD_BLOCK)
    {
        mark_good_block(dst);
        goto done;
    }
#endif

    if (argc > 0 && sscanf(argv[0], "%x", &dst) != 1)
    {
        goto err1;
    }
    if (argc > 1 && sscanf(argv[1], "%x", &len) != 1)
    {
        goto err1;
    }
    if (argc > 2 && sscanf(argv[2], "%x", &src) != 1)
    {
        goto err1;
    }
    else
    {
        if(opcode & OP_PROGRAM)
            force_write = 1;
    }

#ifdef CONFIG_FLASH_CMD_PROGRAM_AFTER_DOWNLOAD_FILE
    if (!force_write && !downloaded && (opcode & OP_PROGRAM))
    {
        printf("Please download file from eth/xmodem before you program flash!\n");
        goto done;
    }
#endif

    printf("dst = 0x%x\n", dst);
    printf("len = 0x%x\n", len);
    printf("src = 0x%x\n", bootvars.load_addr);

#ifdef CONFIG_FLASH_SW_LOCK
    if (opcode & OP_LINFO)
    {
        flash_swlock_info();
        goto done;
    }

    if (opcode & OP_LOCK)
    {
        rc = flash_lock(dst, len);
        if (rc)
        {
            goto fail;
        }
        goto done;
    }

    if (opcode & OP_UNLOCK)
    {
        rc = flash_unlock(dst, len);
        if (rc)
        {
            goto fail;
        }
        goto done;
    }
#endif

    if (opcode & OP_READ)
    {
        printf("flash read: faddr = 0x%x, len = 0x%x, src = 0x%x\n", dst, len, src);
        sf_flush_cache_all();

        rc = flash_control_read_bytes(dst, src, len, CHECK_BAD_BLOCK);
        goto done;
    }

    if (opcode & OP_FULL_ERASE)
    {
        flash_whole_chip_erase();
    }

    if (opcode & OP_ERASE)
    {
        printf("flash erase: addr = 0x%x, len = 0x%x\n", dst, len);
        rc = flash_control_erase(dst, len, CHECK_BAD_BLOCK);
    }

    if (opcode & OP_PROGRAM)
    {
        printf("\nflash program: addr = 0x%x, len = 0x%x, src = 0x%x\n", dst, len, src);
        rc = flash_control_write(dst, src, len, CHECK_BAD_BLOCK);

        // if upgraded from webpage, one-time running firmware automatically
        if (opcode & OP_START_FIRMWARE)
        {
            char *arg[] = { "g", ""};

#ifdef CONFIG_WIFI
            // need to disable Wi-Fi before going to firmware
            pmu_reset_wifi_mac();
#endif
            cmd_go(0, arg + 1);
        }
    }

    if (opcode & OP_WRITE_KEY)
    {
//      write_empty_block_with_otp();
    }

    if (opcode & OP_VERIFY)
    {
#ifdef CONFIG_NAND_FLASH
        printf("Because NAND flash not support direct read,"
               "so not support verify function for NAND flash now\n");
#else
        char buf[16 * 3];
        char *ap[3];

        ap[0] = &buf[0];
        ap[1] = &buf[16];
        ap[2] = &buf[32];
        sprintf(ap[0], "%x", dst + CONFIG_FLASH_BASE);
        sprintf(ap[1], "%x", src);
        sprintf(ap[2], "%x", len);

        printf("flash verify: %s %s %s\n", ap[0], ap[1], ap[2]);
        if ((rc = cmd_cmp(3, ap)))
        {
            goto fail;
        }
#endif
    }

    if (opcode & OP_CDBSAVE)
    {
        cdb_save(0);
    }

    if (opcode & OP_REBOOT)
    {
        char *ap[] = { "3" };
        cmd_rst(1, ap);
    }
  done:
    return ERR_OK;

  fail:
    printf("error:%d\n", rc);
    return rc;

  err1:
    return ERR_PARM;
}

cmdt cmdt_fl __attribute__ ((section("cmdt"))) =
{
    "f", flash_cmd, "f<e,w,r,k,t,v,a,b,c,i><g> <faddr> <len> <buf>;\n"
        "<e>: flash erase\n"
        "<w>: flash program\n"
        "<r>: flash read\n"
        "<k>: flash program recovery image\n"
        "<t>: flash program second image\n"
        "<v>: <e> + <w> + verify memory\n"
        "<a>: <e> + <w>\n"
        "<ag>: <v> + reboot\n"
        "<b>: <e> + <w> + boot cdb save\n"
        "<c>: erase boot cdb\n"
        "<i>: whole chip erase and program flash image\n"
#ifdef BAD_BLOCK_MANAGE
        "<3>: list all bad blocks\n"
        "<4>: mark all blocks in the specified range as bad blocks\n"
        "<5>: mark all blocks in the specified range as good blocks\n"
        "<6>: mark the specified block as bad block\n"
        "<7>: mark the specified block as good block\n"
#endif
};
#endif

#ifdef  CONFIG_CMD_MDIO
/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

#define mdio_rd cm_mdio_rd
#define mdio_wr cm_mdio_wr
int mdio_cmd(int argc, char *argv[])
{
    int set = 0;
    int adr = 0;
    int reg;
    int val;

    if (argc < 1)
    {
        for (adr = 0; adr < 32; adr++)
        {
            // eth phy scan
            printf("phy%d regs", adr);
            for (reg = 0; reg < 32; reg++)
            {
                val = mdio_rd(adr, reg) & 0xffff;
                if ((reg % 8) == 0)
                    printf("\n%02X: ", reg);
                printf("%04x ", val);
            }
            printf("\n");
        }
        return ERR_OK;
    }
    sscanf(argv[0], "%d", &adr);
    if (argc > 1)
    {
        if (1 != sscanf(argv[1], "%x", &reg))
            goto help;
    }
    else
    {
        printf("phy%d regs", adr);
        for (reg = 0; reg < 32; reg++)
        {
            val = mdio_rd(adr, reg) & 0xffff;
            if ((reg % 8) == 0)
                printf("\n%02X: ", reg);
            printf("%04x ", val);
        }
        printf("\n");
        return ERR_OK;
    }

    if (argc > 2)
    {
        if (1 != sscanf(argv[2], "%x", &val))
            goto help;
        mdio_wr(adr, reg, val);
        set++;
    }
    else
    {
        val = mdio_rd(adr, reg) & 0xffff;
    }
    printf("%sphy%d reg 0x%02x=%04x\n", set ? "SET " : "", adr, reg, val);
    return ERR_OK;
  help:
    printf("mdio [adr] [reg] [val]\n\r");
    return ERR_PARM;
}

cmdt cmdt_mdio __attribute__ ((section("cmdt"))) =
{
"mdio", mdio_cmd, "mdio <phy> <reg> <val> ;Access phy register"};

#endif

#ifdef  CONFIG_CMD_CLOCK
/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int clock_cmd(int argc, char *argv[])
{
    int max;
    int d = 1000;
    int i;
    unsigned int start;

    if (argc > 0)
    {
        sscanf(argv[0], "%d", &max);
        if (argc > 1)
            sscanf(argv[1], "%d", &d);
        start = clock_get();
        for (i = 0; i < max; i++)
        {
            mdelay(d);
            printf("%d\n", clock_get());
            if (tstc())
            {
                if (getchar() == 27)
                {
                    printf("user break!");
                    break;
                }
            }
        }
        printf("elapsed time=%d\n", how_long(start));
    }
    else
        printf("tick: %d\n", clock_get());
    return ERR_OK;
}

cmdt cmdt_clock __attribute__ ((section("cmdt"))) =
{
"clock", clock_cmd, "clock [seconds] [period] ;Get clock tick"};

#endif                          //CONFIG_CMD_CLOCK

#ifdef CONFIG_CMD_MEM
/*!
 * function:
 *
 *  \brief
 *  \param c
 *  \return
 */

static int step_sz(char c)
{
    int step;
    if ('b' == c)
        step = 1;
    else if ('h' == c)
        step = 2;
    else
        step = 4;
    return step;
}

/*!
 * function:
 *
 *  \brief
 *  \param addr
 *  \return
 */

int vaddr_check(unsigned long addr)
{
    if (addr < 0x8000000)
        return -1;
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int cmd_cmp(int argc, char *argv[])
{
    unsigned long size, i;
    unsigned char *src, *dst;

    size = 1;
    if (argc < 2)
    {
        goto help;
    }

    if (!hextoul(argv[0], &src))
    {
        goto err1;
    }

    if (!hextoul(argv[1], &dst))
    {
        goto err1;
    }

    if (argc > 2 && !hextoul(argv[2], &size))
    {
        goto err1;
    }

    if (vaddr_check((unsigned long) src) || vaddr_check((unsigned long) dst))
    {
        goto err2;
    }

    for (i = 0; i < size; i++, src++, dst++)
    {
        if (*src != *dst)
        {
            printf("diff @ %x, src = 0x%x, dst = 0x%x \n", i, *src, *dst);
            return ERR_MEM;
        }
    }

    printf("Verify Result: Identical !!!\n");

    return ERR_OK;

  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
  err2:
    return ERR_ADDR;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int mem_copy_cmd(int argc, char *argv[])
{
    int step;
    unsigned long src, dst, size, i;

    step = step_sz(argv[-1][1]);
    size = 1;
    if (argc < 2)
        goto help;
    if (!hextoul(argv[0], &src))
        goto err1;
    if (!hextoul(argv[1], &dst))
        goto err1;
    if (argc > 2 && !hextoul(argv[2], &size))
        goto err1;

    src &= ~(step - 1);
    dst &= ~(step - 1);
    if (vaddr_check(src) || vaddr_check(dst))
        goto err3;
    for (i = 0; i < size; i += step)
    {
        switch (step)
        {
            case 1:
                *((volatile unsigned char *) dst) =
                    *((volatile unsigned char *) src);
                break;
            case 2:
                *((volatile unsigned short *) dst) =
                    *((volatile unsigned short *) src);
                break;
            case 4:
                *((volatile unsigned long *) dst) =
                    *((volatile unsigned long *) src);
                break;
        }
        src += step;
        dst += step;
    }
    return ERR_OK;

  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
  err3:
    return ERR_ADDR;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int mem_dump_cmd(int argc, char *argv[])
{
    unsigned long addr, caddr;
    unsigned long size;
    int step;

    step = step_sz(argv[-1][1]);
    addr = buf_address;
    size = 128;
    if (argc > 0 && !hextoul(argv[0], &addr))
        goto help;
    if (argc > 1 && !hextoul(argv[1], &size))
        goto help;

    if (size > 1024)
        size = 1024;

    addr &= ~(step - 1);
    if (vaddr_check(addr))
        goto err2;
    caddr = addr;
    while (caddr < (addr + size))
    {
        if (caddr % 16 == 0 || caddr == addr)
        {
            printf("\n%08x: ", caddr);
        }
        switch (step)
        {
            case 1:
                printf("%02x ", *((volatile unsigned char *) caddr));
                break;
            case 2:
                printf("%04x ", *((volatile unsigned short *) caddr));
                break;
            case 4:
                printf("%08x ", *((volatile unsigned long *) caddr));
                break;
        }
        caddr += step;
    }
    buf_address = addr + size;
    printf("\n\r");
    return ERR_OK;

  help:
    return ERR_HELP;
  err2:
    return ERR_ADDR;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int mem_enter_cmd(int argc, char *argv[])
{
    int step;
    unsigned long addr, size, val, i;

    step = step_sz(argv[-1][1]);

    size = 1;
    if (argc < 2)
        goto help;

    if (!hextoul(argv[0], &addr))
        goto err1;
    if (!hextoul(argv[1], &val))
        goto err1;
    if (argc > 2 && !hextoul(argv[2], &size))
        goto err1;

    addr &= ~(step - 1);
    if (vaddr_check(addr))
        goto err3;
    for (i = 0; i < size; i += step)
    {
        switch (step)
        {
            case 1:
                *(volatile unsigned char *) (addr + i) = val;
                break;
            case 2:
                *(volatile unsigned short *) (addr + i) = val;
                break;
            case 4:
                *(volatile unsigned long *) (addr + i) = val;
                break;
        }
    }
    return ERR_OK;

  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
  err3:
    return ERR_ADDR;
}

cmdt cmdt_mem[] __attribute__ ((section("cmdt"))) =
{
    {
    "d", mem_dump_cmd, "d<b,h,w> <addr> <size> ;Display memory"}
    ,
    {
    "e", mem_enter_cmd, "e<b,h,w> <addr> <value> <size> ;Enter memory"}
    ,
    {
    "m", mem_copy_cmd, "m<b,h,w> <src> <dst> <size> ;Move memory"}
    ,
    {
    "cmp", cmd_cmp, "cmp <src> <dst> <len> ;Compare"}
,};

#endif

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int urst_cmd(int argc, char *argv[])
{
    printf("\nflash program dst=%x len=%x src=%x\n",
           bootvars.ci_offset, byte_count,
           bootvars.load_addr);

    return ERR_OK;
}

cmdt cmdt_urst __attribute__ ((section("cmdt"))) =
{
"urst", urst_cmd, "upgrade result"};

#ifdef CONFIG_CMD_CACHE
#include <arch/cpu.h>
#define TAGLO_V (1<<7)
#define TAGLO_D (1<<6)
#define TAGLO_L (1<<5)
#define TAGLO_PA(x) (x&0xFFFFFC00)
#define INDEX_SHIFT 5           //32 byte cache line
#define DCACHE_WAY_SHIFT (INDEX_SHIFT+8)        //32 byte cache line + 256 sets
#define ICACHE_WAY_SHIFT (INDEX_SHIFT+9)        //32 byte cache line + 512 sets
#define ADDR2DCACHE_IDX(addr) ((addr&0x00001FE0)>>INDEX_SHIFT)
#define ADDR2ICACHE_IDX(addr) ((addr&0x00003FE0)>>INDEX_SHIFT)
/*!
 * function: show_icache_infos
 *
 *  \brief
 *  \param addr: address
 *  \return
 */
void show_icache_infos(unsigned int addr)
{
    unsigned int taglo, datalo, base, index = ADDR2ICACHE_IDX(addr);
    int w;
    volatile unsigned int *addr_base;
    for (w = 0; w < HAL_ICACHE_WAYS; w++)
    {
        base =
            KSEG0_CACHED_BASE | (w << ICACHE_WAY_SHIFT) | (index <<
                                                           INDEX_SHIFT);
        asm volatile ("cache %0, 0(%1)"::
                      "i" (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28":"=r" (taglo):);
        addr_base =
            (void *) uncached_addr(TAGLO_PA(taglo) | (index << INDEX_SHIFT));
        printf("Set:%d Way:%d tag:%08x (%s %s)\n", index, w, taglo,
               taglo & TAGLO_V ? "V" : "v", taglo & TAGLO_L ? "L" : "l");
        printf("[cache]:");
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 4(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 8(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 12(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 16(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 20(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 24(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 28(%1)"::"i"
                      (HAL_CACHE_OP(ICACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 1":"=r" (datalo):);
        printf("%08x ", datalo);
        printf("\n");
    }
}

/*!
 * function: show_dcache_infos
 *
 *  \brief
 *  \param addr: address
 *  \return
 */
void show_dcache_infos(unsigned int addr)
{
    unsigned int taglo, datalo, base, index = ADDR2DCACHE_IDX(addr);
    int w;
    volatile unsigned int *addr_base;
    for (w = 0; w < HAL_DCACHE_WAYS; w++)
    {
        base =
            KSEG0_CACHED_BASE | (w << DCACHE_WAY_SHIFT) | (index <<
                                                           INDEX_SHIFT);
        asm volatile ("cache %0, 0(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 2":"=r" (taglo):);

        addr_base =
            (void *) uncached_addr(TAGLO_PA(taglo) | (index << INDEX_SHIFT));
        printf("Set:%d Way:%d tag:%08x (%s %s %s)\n",
               index,
               w,
               taglo,
               taglo & TAGLO_V ? "V" : "v",
               taglo & TAGLO_D ? "D" : "d", taglo & TAGLO_L ? "L" : "l");
        printf("[cache]:");
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 4(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 8(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 12(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 16(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 20(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 24(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);

        asm volatile ("cache %0, 28(%1)"::"i"
                      (HAL_CACHE_OP(DCACHE, INDEX_LOAD_TAG)), "r"(base));
        asm volatile ("mfc0 %0, $28, 3":"=r" (datalo):);
        printf("%08x ", datalo);
        printf("\n");
        if (taglo & TAGLO_D)
            printf("[ncach]:%08x %08x %08x %08x %08x %08x %08x %08x\n",
                   *addr_base, *(addr_base + 1), *(addr_base + 2),
                   *(addr_base + 3), *(addr_base + 4), *(addr_base + 5),
                   *(addr_base + 6), *(addr_base + 7));
    }
}

/*!
 * function: cache_cmd
 *
 *  \brief a command to diagnose cache line state
 *  \param argc
 *  \param argv
 *  \return
 */
int cache_cmd(int argc, char *argv[])
{
    unsigned int addr, size;
    if (argc < 1 || argc > 4)
        goto help;

    if (argc > 1 && !hextoul(argv[1], &addr))
        goto err1;

    if (argv[0][0] == 'i')
    {
        if (argv[0][1] == 'i')
        {
            printf("icache invalidate ");
            if ((argc == 3) && hextoul(argv[2], &size))
            {
                printf(" %08x~%08x\n", addr, addr + size);
                HAL_ICACHE_INVALIDATE(addr, size);
            }
            else
            {
                printf(" all\n");
                HAL_ICACHE_INVALIDATE_ALL();
            }
        }
        else
            show_icache_infos(addr);
    }
    else if (argv[0][0] == 'd')
    {
        if (argv[0][1] == 'f')
        {
            printf("dcache flush");
            if ((argc == 3) && hextoul(argv[2], &size))
            {
                printf(" %08x~%08x\n", addr, addr + size);
                HAL_DCACHE_FLUSH(addr, size);
            }
            else
            {
                printf(" all\n");
                HAL_DCACHE_FLUSH_ALL();
            }
        }
        else if (argv[0][1] == 'i')
        {
            printf("dcache invalidate");
            if ((argc == 3) && hextoul(argv[2], &size))
            {
                printf(" %08x~%08x\n", addr, addr + size);
                HAL_DCACHE_INVALIDATE(addr, size);
            }
            else
            {
                printf(" Plz add <addr> <size>\n");
                goto err1;
            }
        }
        else
            show_dcache_infos(addr);
    }
    else
        goto err1;

    return ERR_OK;
  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
}

cmdt cmdt_cache __attribute__ ((section("cmdt"))) =
{
"cache", cache_cmd, "cache <i,d><f,i> <addr> <size>"};
#endif

#ifdef CONFIG_CMD_SECURE_MODE
/*!
 * function: se_cmd
 *
 *  \brief The command to test different secure mode
 *  \param argc
 *  \param argv
 *  \return
 */
int se_cmd(int argc, char *argv[])
{
    if (argc != 1)
    {
        goto help;
    }

    if (!strcmp("none", argv[0]))
    {
        printf("Change Secure Method: None\n");
        change_aes_enable_status(DISABLE_SECURE);
    }
    else if (!strcmp("aes", argv[0]))
    {
        printf("Change Secure Method: Use AES key\n");
        change_aes_enable_status(ENABLE_SECURE_REG);
    }
    else if (!strcmp("otp", argv[0]))
    {
        printf("Change Secure Method: Use OTP key\n");
        change_aes_enable_status(ENABLE_SECURE_OTP);
    }
    else
    {
        goto err1;
    }

    return ERR_OK;
  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
}

cmdt cmdt_se __attribute__ ((section("cmdt"))) =
{
"se", se_cmd, "se <none, aes, otp> ;change secure method"};
#endif

#if 0
#include <mt_types.h>
struct bb_regs_ahb {
        int group;
        u8 num;
        u8 val;
};
struct bb_regs_ahb bb_init_tbl_r88_panther_ahb [] = {
    {0, 0x00, 0x01},  // reset BB to default value
        /* reg 0x01 MUST at second entry */
        {0, 0x01, 0x80},  // toggle RXHP and old bb_LMAC interface
        {0, 0xF4, 0x00},
    {0, 0x02, 0x33},  // TX IQ swap: V7 is different with V5
    {0, 0xf2, 0x80},  // DAC CLK 40MHz
    {0, 0xf3, 0x22},  // ADC CLK 40MHz
    {0, 0x54, 0x2b},  // disable on-fly IQmismatch compenstion (0x23 enable)
        {0, 0x05, 0x80},  // To pass TX mask of B mode
        {1, 0x0, 0x9c},//AGC table for Panther & Panther rf
        {1, 0x1, 0x66},
        {1, 0x2, 0x30},
        {1, 0x3, 0x7d},
        {1, 0x4, 0x30},
        {1, 0x5, 0x42},
        {1, 0x6, 0x8f},
        {1, 0x7, 0x23},
        {1, 0x8, 0x66},
        {1, 0x9, 0x71},
        {1, 0x0a, 0x18},
        {1, 0x0b, 0xbd},
        {1, 0x0c, 0x0b},
        {1, 0x0d, 0x2f},
        {1, 0x0e, 0x30},
        {1, 0x0f, 0x04},
        {1, 0x10, 0x5b},
        {1, 0x11, 0x08},
        {1, 0x12, 0xc8},
        {1, 0x13, 0x00},
        {1, 0x14, 0x00},
        {1, 0x16, 0x96},
        {1, 0x17, 0x1a},
        {1, 0x40, 0x7f},
};

u8 bb_register_read(int group,u32 bb_reg)
{
    u8 value=0;
    u32 data, dst;

    if(group == 0)
    {
        dst = (bb_reg/4) * 4;
    }
    else if(group == 1)
    {
        dst = ((bb_reg/4) * 4) + 0x100;
    }
    else if(group ==2)
    {
        dst = ((bb_reg/4) * 4) + 0x200;
    }
    else if(group==3)
    {
        dst = ((bb_reg/4) * 4) + 0x300;
    }
    else
    {
        printf("XXX: UnKonwn group %d\n", group);
        return 0;
    }
    data = BBREG_READ32(dst);

    value = (data >> 8*(bb_reg % 4)) & 0xffUL;

    //printf("\t\t\t\tread %d\t%08x\t%02x\n", group, bb_reg, value);
    return value;
}

void bb_register_write(int group, u32 bb_reg, u8 value)
{
//    printf("\t\t\t\twrite %d\t%08x\t%02x\n", group, bb_reg, value);
    u32 data, dst, mask=0;

    if(group==0)
    {
        dst = (bb_reg/4) * 4;
    }
    else if(group==1)
    {
        dst = ((bb_reg/4) * 4) + 0x100;
    }
    else if(group==2)
    {
        dst = ((bb_reg/4) * 4) + 0x200;
    }
    else if(group==3)
    {
        dst = ((bb_reg/4) * 4) + 0x300;
    }
    else
    {
        printf("XXX: UnKonwn group %d\n", group);
        return;
    }
    data =BBREG_READ32(dst);

//    printf("read %08x\t%08x ", dst, data);
    if(bb_reg%4==0)
    {
        mask |= 0x000000ffUL;
    }
    else if(bb_reg%4==1)
    {
        mask |= 0x0000ff00UL;
    }
    else if(bb_reg%4==2)
    {
        mask |= 0x00ff0000UL;
    }
    else if(bb_reg%4==3)
    {
        mask |= 0xff000000UL;
    }
    data = ( (data & ~(mask)) | ((value << 8*(bb_reg % 4)) & (mask)) );
    // reg0 will reset BB by write any value except zero
    // write the reg1,2,3 with the LSByte==0, so the BB won't be reset
    if(((dst==0) && (group==0)) && (bb_reg!=0))
    {
        mask = 0xffffff00UL;
        data = data & mask;
    }

//    printf("write %08x\t%08x\n", dst ,data);

    BBREG_WRITE32(dst, data);
}

#define TOTAL_REG_CNT   (17 + 1 + 5)
void backup_bb_result(u8 restore_data[TOTAL_REG_CNT])
{
	u32 i, reg_addr;
	/* save BB20~30,33,5A~5E */
	i=0;
	for (reg_addr=0x20UL; reg_addr<=0x30UL; reg_addr++)
	{
		restore_data[i] = bb_register_read(0, reg_addr);
		//DBG_PRINTF(INFO, "save BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}
	restore_data[i] = bb_register_read(0, 0x33);
	//DBG_PRINTF(INFO, "save BBreg33 data = %02x\n", restore_data[i]);
	i++;
	for (reg_addr=0x5aUL; reg_addr<=0x5eUL; reg_addr++)
	{
		restore_data[i] = bb_register_read(0, reg_addr);
		//DBG_PRINTF(INFO, "save BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}
	//DBG_PRINTF(INFO, "save BBreg data cnt = %d\n", i);
}

void restore_bb_result(u8 restore_data[TOTAL_REG_CNT])
{
	u32 i, reg_addr;
	/* restore BB20~30,33,5A~5E after reset BB */
	i=0;
	for (reg_addr=0x20UL; reg_addr<=0x30UL; reg_addr++)
	{
		bb_register_write(0, reg_addr, restore_data[i]);
		//DBG_PRINTF(INFO, "restore BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}
	bb_register_write(0, 0x33, restore_data[i]);
	//DBG_PRINTF(INFO, "restore BBreg33 data = %02x\n", restore_data[i]);
	i++;
	for (reg_addr=0x5aUL; reg_addr<=0x5eUL; reg_addr++)
	{
		bb_register_write(0, reg_addr, restore_data[i]);
		//DBG_PRINTF(INFO, "restore BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}

}

void bb_init(void)
{
	u32 j, sz;
	u8 restore_data[TOTAL_REG_CNT];

/*
	if(0 == ldev->bb_reg_tbl) {
		serial_printf("No BB init table ??\n");
		return;
	}

	init_tbl = ((struct bb_rev_t *)(ldev->bb_reg_tbl))->tbl;
	sz = ((struct bb_rev_t *)(ldev->bb_reg_tbl))->size;
*/

	backup_bb_result(restore_data);
        sz = (sizeof(bb_init_tbl_r88_panther_ahb)/sizeof(struct bb_regs_ahb));
	// reset BB
	if (0 < sz)
	{
		bb_register_write(bb_init_tbl_r88_panther_ahb[0].group, bb_init_tbl_r88_panther_ahb[0].num, bb_init_tbl_r88_panther_ahb[0].val);
	}
	restore_bb_result(restore_data);
	for(j = 1; j < sz; j++)
	{
//		bb_register_write(0, init_tbl[j].num, init_tbl[j].val);
            bb_register_write(bb_init_tbl_r88_panther_ahb[j].group, bb_init_tbl_r88_panther_ahb[j].num, bb_init_tbl_r88_panther_ahb[j].val);
	}
}
#endif

#ifdef CONFIG_CMD_INIT
/*!
 * function: init_cmd
 *
 *  \brief The command to initialize module manually
 *  \param argc
 *  \param argv
 *  \return
 */
int init_cmd(int argc, char *argv[])
{
    int f_init_type = 0;
    int savecdb = 0;
    
    if (argc < 1)
    {
        goto help;
    }

//  if (!strcmp("bb", argv[0]))
//  {
//      bb_init();
//  }

    if (!strcmp("flash", argv[0]))
    {
        if (argc != 2)
        {
            goto help;
        }

        if(!strcmp("0", argv[1]) || !strcmp("otp", argv[1]))
        {
            f_init_type = otp_get_boot_type();
        }

        // parse flash initialized type
        if (!strcmp("1", argv[1]) || !strcmp("nor", argv[1]))
        {
            f_init_type = 1;
        }

        if (!strcmp("2", argv[1]) || !strcmp("nand", argv[1]))
        {
            f_init_type = 2;
        }

        if (!strcmp("3", argv[1]) || !strcmp("nand_otp", argv[1]))
        {
            f_init_type = 3;
        }

        if (f_init_type != BOOT_FROM_NOR
            && f_init_type != BOOT_FROM_NAND
            && f_init_type != BOOT_FROM_NAND_WITH_OTP)
        {
            goto err1;
        }
    
        otp_read_config();
        flash_init(f_init_type);
    }
    else if (!strcmp("cdb", argv[0]))
    {
        if (argc != 1)
        {
            goto help;
        }
        
        config_init(&savecdb);
    }
#ifdef CONFIG_DHCPD
    else if (!strcmp("dhcp", argv[0]))
    {
        if (argc != 1)
        {
            goto help;
        }

        dhcpd_init();
    }
#endif
    else
    {
        goto err1;
    }

    return ERR_OK;
  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;
}

cmdt cmdt_init __attribute__ ((section("cmdt"))) =
{
"init", init_cmd, "init <flash> <otp(0), nor(1), nand(2), nand_otp(3)>\n"
                  "     1: boot from NOR, 2: boot from NAND, 3: boot from NAND with OTP\n"
                  "init <cdb>\n"
#ifdef CONFIG_DHCPD
                  "init <dhcp>\n"
#endif
};
#endif

