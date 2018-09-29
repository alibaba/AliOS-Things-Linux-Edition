/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file sflash_Test.c
*   \brief test functions
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#if defined(SIM)
#define sflash_udelay(n) cosim_delay_ns( (n) * 1000)
#else
#define sflash_udelay(n) {int _www; for (_www=0; _www< 100; _www++); }
#endif

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
unsigned char key[] = { 
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

unsigned char buf[] = { 
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

unsigned char buf1[32];

unsigned char exp_data[] = { 
    0x70, 0xd2, 0x36, 0xb3, 0xd6, 0x32, 0x1e, 0x50, 0x89, 0x20, 0xfa, 0x62, 0x0d, 0x83, 0x32, 0x8b,
    0x63, 0x04, 0x23, 0x44, 0xf7, 0xc5, 0xe1, 0xa5, 0x36, 0xfb, 0x22, 0x7b, 0x9c, 0x0f, 0x48, 0xb5, 
};

unsigned char otpkey[] = { 
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};
//	unsigned char otpkey[] = { 
//	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
//	};
unsigned char otpkey_r[32];

unsigned char otp_buf[] = { 
    0x00, 0x90, 0x04, 0x3c, 0x40, 0x27, 0x84, 0x24,
    0x00, 0x90, 0x03, 0x3c, 0x00, 0x29, 0x63, 0x24,
    0x00, 0x00, 0x80, 0xac, 0x04, 0x00, 0x80, 0xac,
    0x08, 0x00, 0x80, 0xac, 0x0c, 0x00, 0x80, 0xac
};

unsigned char otp_exp_data[] = { 
    0x68, 0xc4, 0xbf, 0x2d, 0x8f, 0xf8, 0x15, 0x41,
    0x28, 0xd4, 0xaa, 0x4e, 0x1e, 0x66, 0x6f, 0x7d,
    0xb9, 0xaf, 0x16, 0x01, 0x7f, 0x86, 0xbc, 0x2d,
    0x61, 0xb4, 0xf5, 0x47, 0x4e, 0x09, 0x57, 0x67
};

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/    
void nand_read_write_test(void)
{
    sf_log("read_write_test() for NAND !!\n");
    
    int blockNum = 4;
    int pageNum = 2;
    int input[8] = {0x12345678, 0x23456789, 0x3456789a, 0x456789ab,
                    0x56789abc, 0x6789abcd, 0x789abcde, 0x89abcdef};
    int output[8];
    int size = 32;
    u32 block_size = get_block_size();
    u32 page_size = get_page_size();
    int test_base = block_size * 5;
    
    int i = 0;
    int j = 0;  
    int k = 0;
    int rc = 0;

    change_aes_enable_status(DISABLE_SECURE);
	
    sf_log("flash_erase()\n");
    flash_erase(test_base, (blockNum * block_size)/2, CHECK_BAD_BLOCK);
    
    sf_log("flash_write()\n");
    for (i = 0; i < blockNum; i++)
    {
        for (j = (pageNum - 1); j < pageNum; j++)
        {
            flash_write(test_base + j * page_size * 2 + i * block_size, (u32)input, size, CHECK_BAD_BLOCK);
        }
    }
    
    sf_log("flash_read()\n");
    for (i = 0; i < blockNum; i++)
    {
        for (j = (pageNum - 1); j < pageNum; j++)
        {
            rc = flash_read_bytes(test_base + j * page_size * 2 + i * block_size, (u32)output, size, CHECK_BAD_BLOCK);

            for (k = 0; k < size/4; k++)
            {
                sf_log("rc = 0x%x\n", output[k]);
            }
        }
    }
}

// for test one block
void nand_read_write_test_2(void)
{
    sf_log("read_write_test() for NAND !!\n");
    
    int block_index = 0;
    int input[8] = {0x12345678, 0x23456789, 0x3456789a, 0x456789ab,
                    0x56789abc, 0x6789abcd, 0x789abcde, 0x89abcdef};
    int output[8];
    int size = 32;
    u32 block_size = get_block_size();
    u32 page_size = get_page_size();
    int test_base = 4096 + 64;
    int j = 0;
    int rc = 0;

    change_aes_enable_status(DISABLE_SECURE);
	
    sf_log("flash_erase()\n");
    flash_erase(block_index * block_size, block_size/2, CHECK_BAD_BLOCK);
    
    sf_log("flash_write()\n");
    rc = flash_write(test_base + block_index*block_size, (u32)input, size, CHECK_BAD_BLOCK);

    sf_log("flash_read()\n");
    rc = flash_read_bytes(test_base + block_index*block_size, (u32)output, size, CHECK_BAD_BLOCK);
    for (j = 0; j < size/4; j++)
    {
        sf_log("output = 0x%x\n", output[j]);
    }
}

void nor_read_write_test(void)
{
    sf_log("read_write_test() for NOR !!\n");
    
    int blockNum = 0;
    int pageNum = 1;
    int abc[4] = {0x11223344, 0x22334455, 0x33445566, 0x44556677};
    int ab[8] = {0x12345678, 0x23456789, 0x3456789a, 0x456789ab,
                 0x56789abc, 0x6789abcd, 0x789abcde, 0x89abcdef};
    int output[8];
    int size = 32;
    int test_base = 0x00;
    
    int i = 0;
    int j = 0;  
    int rc = 0;
	
    change_aes_enable_status(DISABLE_SECURE);
    
    sf_log("flash_erase()\n");
    flash_erase((64 * blockNum << 12), 1, CHECK_BAD_BLOCK);
    
    sf_log("flash_write()\n");
    for (i = 0; i < pageNum; i++)
    {
        flash_write(test_base + j + i * 0x20, (u32)ab, size, CHECK_BAD_BLOCK);
    }
    
    sf_log("flash_read()\n");
    for (i = 0; i < pageNum; i++)
    {
        rc = flash_read_bytes(test_base + j + i * 0x20, (u32)output, size, CHECK_BAD_BLOCK);
        for (j = 0; j < size/4; j++)
        {
            sf_log("rc = 0x%x\n", output[j]);
        }
    }
}
	
void download_boot1_boot2_from_xmodem(void)
{
    unsigned int block_num = 0;
    unsigned int offset = 0;

    offset = block_num * get_block_size();
    
    flash_erase(offset + BOOT1_PAGE_START, BOOT1_SIZE + BOOT2_SIZE, CHECK_BAD_BLOCK);

//	    change_aes_enable_status(ENABLE_SECURE_REG);
    change_aes_enable_status(DISABLE_SECURE);
    
    sf_log("burn boot1 into flash\n");
    xmodem_rx((void *) BOOT1_BURN_BUF, BOOT1_SIZE);
    burn_boot1_into_flash(offset);

    sf_log("burn boot2 with lzma into flash\n");
    xmodem_rx((void *) BOOT2_BURN_BUF, BOOT2_SIZE);
    burn_boot2_into_flash(offset);
}

void download_linux_from_xmodem(void)
{
    int block_num = 0;

    flash_erase(get_block_size() * BOOT_MID_BLOCK_NUM, 0x210000, CHECK_BAD_BLOCK);
    
    change_aes_enable_status(ENABLE_SECURE_REG);
    xmodem_rx((void *) BOOT2_BASE, 0x210000);
    burn_linux_into_flash();
}

int compare_with_expected_data(unsigned char *input_data, unsigned char *_exp_data)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        if (input_data[i] != _exp_data[i])
        {
            return -1;
        }
    }
    return 0;
}

int regkey_read_write_test(void)
{
    sf_log("read_write_test() for NAND !!\n");
    
    int size = 32;
    int test_base = 0x0;    
    int i = 0;
    int rc = 0;
    int ret;

    sf_log("flash_erase()\n");
    flash_erase(0, size, CHECK_BAD_BLOCK);
    
    sf_log("Write data\n");
    for (i = 0; i < size; i++)
    {
        sf_log("%02x ", buf[i]);
    }
    sf_log("\n");

    change_aes_enable_status(ENABLE_SECURE_REG);

    sf_log("flash_write()\n");
    for (i = 0; i < size; i += size)
    {
        flash_write(test_base + i, (u32) &buf[0], size, CHECK_BAD_BLOCK);
    }
    
    change_aes_enable_status(DISABLE_SECURE);

    sf_log("flash_read()\n");
    for (i = 0; i < size; i += size)
    {
        rc = flash_read_bytes(test_base + i, (u32) &buf1[0], size, CHECK_BAD_BLOCK);
    }

    sf_log("Read data\n");
    for (i = 0; i < size; i++)
    {
        sf_log("%02x ", buf1[i]);
    }
    sf_log("\n");

    ret = compare_with_expected_data(buf1, exp_data);

    change_aes_enable_status(ENABLE_SECURE_REG);

    sf_log("flash_read()\n");
    for (i = 0; i < size; i += size)
    {
        rc = flash_read_bytes(test_base + i, (u32) &buf1[0], size, CHECK_BAD_BLOCK);
    }

    sf_log("Read data\n");
    for (i = 0; i < size; i++)
    {
        sf_log("%02x ", buf1[i]);
    }
    sf_log("\n");

    return ret;
}


int otpkey_read_write_test(void)
{
    sf_log("read_write_test() for NAND !!\n");

    int size = 32;
    int test_base = 0x0;    
    int i = 0;
    int rc = 0;
    int ret;

    sf_log("flash_erase()\n");
    flash_erase(0, size, CHECK_BAD_BLOCK);
    
    sf_log("Write data\n");
    for (i = 0; i < size; i++)
    {
        sf_log("%02x ", otp_buf[i]);
    }
    sf_log("\n");

    change_aes_enable_status(ENABLE_SECURE_OTP);

    sf_log("flash_write()\n");
    for (i = 0; i < size; i += size)
    {
        flash_write(test_base + i, (u32) &otp_buf[0], size, CHECK_BAD_BLOCK);
    }
    
    change_aes_enable_status(DISABLE_SECURE);

    sf_log("flash_read()\n");
    for (i = 0; i < size; i += size)
    {
        rc = flash_read_bytes(test_base + i, (u32) &buf1[0], size, CHECK_BAD_BLOCK);
    }

    sf_log("Read data\n");
    for (i = 0; i < size; i++)
    {
        sf_log("%02x ", buf1[i]);
    }
    sf_log("\n");

    ret = compare_with_expected_data(buf1, otp_exp_data);

    change_aes_enable_status(ENABLE_SECURE_OTP);

    sf_log("flash_read()\n");
    for (i = 0; i < size; i += size)
    {
        rc = flash_read_bytes(test_base + i, (u32) &buf1[0], size, CHECK_BAD_BLOCK);
    }
    
    sf_log("Read data\n");
    for (i = 0; i < size; i++)
    {
        sf_log("%02x ", buf1[i]);
    }
    sf_log("\n");

    return ret;
}


void otp_test(void)
{
    int i;
    int test_failed = 0;
	
#if 0  // AES test with key in the register
    pdma_program_aes_key(key, 32);
    PDMAREG(PDMA_AES_CTRL) = (PDMA_AES_MODE_CBC | PDMA_AES_KEYLEN_256);

    if (0 == regkey_read_write_test())
    {
       sf_log("R/W test with AES register key passed.\n");
    }
    else
    {
       test_failed = 1;
    }
#endif

#if 0   // AES test with OTP key
    //  program AES key into OTP   
    efuse_write(otpkey, 0, 32);

    //  read OTP key to fetch into hardware
    efuse_read(otpkey_r, 0, 32);
//	    sf_log("otpkey_r\n");
//	    for (i = 0; i < 32; i++)
//	    {
//	        sf_log("%02x ", otpkey_r[i]);
//	    }
//	    sf_log("\n");
   
    //  AES test with key in the OTP memory
    if (0 == otpkey_read_write_test())
    {
       sf_log("R/W test with AES OTP key passed.\n");
    }
    else
    {
       test_failed = 1;
    }
#endif

#if 1
    sf_log("PDMA_AES_CTRL2 test\n");

    sf_log("Orig. value: %08x\n", PDMAREG(PDMA_AES_CTRL2));

    PDMAREG(PDMA_AES_CTRL2) = PDMA_AES_OTPKEY_ENC_DISABLE;
    sf_log("After write 1: %08x\n", PDMAREG(PDMA_AES_CTRL2));

    PDMAREG(PDMA_AES_CTRL2) = 0;
    sf_log("After write 0: %08x\n", PDMAREG(PDMA_AES_CTRL2));

    if (0 != otpkey_read_write_test())
    {
       /* expect to fail on R/W test, as OTP key encryption is disabled */
       sf_log("R/W test with AES OTP key disabled passed.\n");
    }
    else
    {
       test_failed = 1;
    }
#endif

    if (test_failed == 1)
    {
        sf_log("Secure boot test failed......");
    }
    else
    {
        sf_log("Test Passed......");
    }
}

//	void otp_set_test_key(void)
//	{
//	    // register key
//	    pdma_program_aes_key(otpkey, 32);
//	    PDMAREG(PDMA_AES_CTRL) = (PDMA_AES_MODE_CBC | PDMA_AES_KEYLEN_256);
//	
//	    // otp key
//	    efuse_write(otpkey, 0, 32);
//	}

void nand_test(void)
{
    sf_log("Flash_Control: nand_test()\n");

//	    otp_test();
//	    otp_set_test_key();

//	    nand_read_write_test();
//	    nand_read_write_test_2();
	
//	    download_boot1_boot2_from_xmodem();
//	    download_linux_from_xmodem();

//	    boot_from_flash();
//	    load_boot2();
}

