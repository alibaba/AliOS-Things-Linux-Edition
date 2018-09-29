#
# Makefile
#
# CONFIGURATION PART

# 4ADDR 32MB NOR support
#NOR32M=1

# Customized NAND support
#NANDOTP=1

#BOOT1_24K=-DBOOT1_24K
#MEM_TEST=1
USB=1
#PCM_I2S=1
#I2CS=1
#SDHC=1
MINI_SDHC=1
#PWM=1
#WIFI=1
#SCHED=1
#FPGA=1
#ASIC_DDR3=1
UART=1
FATFS=1
#VERBOSE_DEBUG=1
DHCP=1
#MASS_PRODUCTION_TEST=1

#BOOT1_MINI_SDHC=1
BOOT1_UART=1

#TEST_MODE_BOARD=1			# the board with Panther ASIC in testmode + MT301

#MPCFG=1                                # mass-production configuration, set default bootvars to MP
#BOOT3_LCD=1

# END OF CONFIGURATION PART

BOOT_REVNUM:=$(shell ./setlocalversion)
B2_ZIP=boot2.bin.lzo
B3_ZIP=boot3.bin.lzma
BT_LDS=include/boot.lds
IN_CACHE=1

# TEXT ADDRESSES
STR_TEXT = 0x90000000
B1_FERASE_TEXT = 0xB0000000
B1_DEBRICK_TEXT = 0xB0000000
B1_TEXT = 0x90000000
B2_TEXT = 0x83800000

HEAD_LEN = 48
B3_TEXT = 0x83000000

# CROSS-COMPILER PREFIX
# build little-endian binary by default 
ENDIAN=-EL
CROSS_PREFIX ?= mipsel-alios-linux-

ifeq ($(LE),1)
ENDIAN=-EL
CROSS_PREFIX ?= mipsel-alios-linux--
endif

ifeq ($(BE),1)
ENDIAN=-EB
CROSS_PREFIX ?= mipsel-alios-linux-
endif

# COMPILER FLAGS
CPU = -mips32r2
CFLAGS = -Wuninitialized -Wall -Werror $(CPU) $(ENDIAN) -g -include config.h -I. -I./include
ifdef NOR32M
CFLAGS += -DNOR_4ADDR_MODE
endif
ifdef NANDOTP
CFLAGS += -DNAND_OTP_MODE
endif
ifdef FPGA
CFLAGS += -DCONFIG_FPGA
else
ifdef ASIC_DDR3
CFLAGS += -DCONFIG_USE_DDR3
else
CFLAGS += -DCONFIG_USE_DDR2
endif
ifdef TEST_MODE_BOARD
CFLAGS += -DTEST_MODE_BOARD
endif
endif
ifdef DEBRICK
CFLAGS += -DDEBRICK
endif
ifdef VERBOSE_DEBUG
CFLAGS += -DVERBOSE_DEBUG
endif
CFLAGS += -fno-builtin
CFLAGS += -fno-common
CFLAGS += -fno-pic
CFLAGS += -fno-strict-aliasing -fregmove -Os
CFLAGS += -msoft-float
ifdef WIFI
CFLAGS += -G0
else
CFLAGS += -mno-abicalls
endif
CFLAGS += -DBOOT_REVNUM=\"$(BOOT_REVNUM)\" -DB1_TEXT=$(B1_TEXT) -DB2_TEXT=$(B2_TEXT)
ifneq ($(LE),1)
CFLAGS += -DBIG_ENDIAN
else
CFLAGS += -DLITTLE_ENDIAN
endif
CFLAGS += -std=gnu90
CFLAGS_src/start.S = -Wa,-gstabs
ifdef USB
CFLAGS += -DCONFIG_USB -DCONFIG_USB_LOOPBACK_TEST
#B2_TEXT = 0xa1c00000
endif
ifdef I2CS
B2_IN_SRAM=1
CFLAGS += -DCONFIG_I2CS -DCONFIG_SDHC
endif
ifdef PCM_I2S
CFLAGS += -DCONFIG_PCM_I2S
endif
ifdef SDHC
CFLAGS += -DCONFIG_SDHC
endif
CFLAGS_src/str/str_debug.c = -mips16
CFLAGS_src/boot1_lzo.c = -mips16
CFLAGS_src/boot1_debrick.c = -mips16
CFLAGS_src/sflash/pdma_driver_boot1.c = -mips16
CFLAGS_src/sflash/sflash_controller_boot1.c = -mips16
CFLAGS_src/sflash/sflash_controller_boot1_ferase.c = -mips16
CFLAGS_src/ddr/ddr3_cal.c = -mips16
CFLAGS_src/ddr/ddr2_cal.c = -mips16
CFLAGS_src/ddr/ddr3_fpga_clbr.c = -mips16
ifdef MINI_SDHC
CFLAGS += -DCONFIG_MINI_SDHC
CFLAGS_src/mmc/mini-sdhc-ipl_boot1.c = -mips16
endif
ifdef PWM
CFLAGS += -DCONFIG_PWM
endif
ifdef MPCFG
CFLAGS += -DMPCFG
endif
ifdef BOOT3_LCD
#CFLAGS += -DCONFIG_LCD_ST7789V -DCONFIG_GDMA -DLCD_PDMA
CFLAGS += -DCONFIG_LCD_ST7789V -DLCD_PDMA
endif
CFLAGS += -DCONFIG_PANTHER
ifdef UART
CFLAGS += -DCONFIG_UART
CFLAGS_src/xmodem_boot1.c = -mips16
endif
ifdef FATFS
CFLAGS_src/fatfs/ff.c = -mips16
CFLAGS_src/fatfs/ffsystem.c = -mips16
endif

ifdef IN_CACHE
B1_TEXT := $(B1_TEXT:0xa%=0x8%)
B1_TEXT := $(B1_TEXT:0xb%=0x9%)
B2_TEXT := $(B2_TEXT:0xa%=0x8%)
B2_TEXT := $(B2_TEXT:0xb%=0x9%)
endif
FGS = $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(filter %.c %.S, $<))

# LINKER FLAGS
LDFLAGS_STR = -e boot -Ttext=$(STR_TEXT) $(ENDIAN) -Bstatic
LDFLAGS_B1_FERASE = -e boot -Ttext=$(B1_FERASE_TEXT) $(ENDIAN) -Bstatic
LDFLAGS_B1_DEBRICK = -e boot -Ttext=$(B1_DEBRICK_TEXT) $(ENDIAN) -Bstatic
LDFLAGS_B1 = -e boot -Ttext=$(B1_TEXT) $(ENDIAN) -Bstatic
LDFLAGS_B2 = -e start -Ttext=$(B2_TEXT) $(ENDIAN) -Bstatic
LDFLAGS_B3 = -e start -Ttext=$(B3_TEXT) $(ENDIAN) -Bstatic

# STR objects
ifdef FPGA
STR_OBJS = src/str/str_boot.o src/str/str.o src/str/str_debug.o
else
STR_OBJS = src/str/str_boot.o src/str/str.o src/str/str_debug.o src/ddr/ddr_srf.o
endif

# BOOT1_DEBRICK/BOOT1 common objects
B1_COMMON_OBJS = src/boot.o
B1_COMMON_OBJS += src/sflash/otp_boot1.o

# if SDHC is enabled in boot1, sflash & uart function is removed
ifdef BOOT1_MINI_SDHC
CFLAGS += -DCONFIG_BOOT1_MINI_SDHC
B1_COMMON_OBJS += src/mmc/mini-sdhc-ipl_boot1.o
else
B1_COMMON_OBJS += src/sflash/pdma_driver_boot1.o src/sflash/sflash_controller_boot1.o
ifdef BOOT1_UART
CFLAGS += -DCONFIG_BOOT1_UART
B1_COMMON_OBJS += src/xmodem_boot1.o src/serial_boot1.o
endif
endif

ifdef FPGA
B1_COMMON_OBJS += src/ddr/ddr3_fpga_clbr.o
else
ifdef ASIC_DDR3
B1_COMMON_OBJS += src/ddr/ddr3_cal.o
else
B1_COMMON_OBJS += src/ddr/ddr2_cal.o
endif
endif
B1_COMMON_OBJS += src/cpu_cache.o

# BOOT1 flash erase objects
B1_FERASE_OBJS = src/boot.o
B1_FERASE_OBJS += src/sflash/otp_boot1.o
B1_FERASE_OBJS += src/cpu_cache.o
B1_FERASE_OBJS += src/sflash/sflash_controller_boot1_ferase.o
B1_FERASE_OBJS += src/clock_boot1.o
B1_FERASE_OBJS += src/boot1_ferase.o

# BOOT1 debrick objects
B1_DEBRICK_OBJS = $(B1_COMMON_OBJS)
B1_DEBRICK_OBJS += src/clock_boot1.o
B1_DEBRICK_OBJS += src/boot1_debrick.o

# BOOT1 objects
B1_OBJS = $(B1_COMMON_OBJS)
B1_OBJS += src/clock_boot1.o
B1_OBJS += src/boot1_lzo.o

# BOOT2 objects

B2_OBJS = src/start.o

ifndef FPGA
B2_OBJS += src/ddr/ddr_srf.o
endif

B2_OBJS += src/cpu_cache.o
#B2_OBJS += src/mt/init_vpe1.o
B2_OBJS += src/libio.o src/libstr.o src/memcpy.o src/cmd_proc.o
B2_OBJS += src/cdb.o src/cmd.o src/cmd2.o
B2_OBJS += src/trap.o src/irq.o
B2_OBJS += src/netcore.o src/netprot.o
B2_OBJS += src/cpu.o
B2_OBJS += src/main.o
B2_OBJS += src/pinmux.o
B2_OBJS += src/pmu.o
B2_OBJS += src/gpio_driver_strength.o
B2_OBJS += src/sflash/otp.o
B2_OBJS += src/sflash/pdma_driver_boot2.o src/sflash/sflash_controller_boot2.o
B2_OBJS += src/gpio_fset.o
B2_OBJS += src/hw.o src/clock.o src/serial.o
B2_OBJS += src/cm_mac.o
B2_OBJS += src/webserv.o
B2_OBJS += src/telnetd.o
B2_OBJS += src/image.o src/md5.o
B2_OBJS += src/led.o
B2_OBJS += src/dhry_1.o src/dhry_2.o
B2_OBJS += src/gdma.o
B2_OBJS += src/asyncfifo.o
B2_OBJS += src/tsi.o
B2_OBJS += src/tsi_test.o
#B2_OBJS += src/gspi/spi.o       # created for SDIO-SPI test
#B2_OBJS += src/gspi/gspi.o
#B2_OBJS += src/ddr/ddr3_fpga_clbr.o

ifdef MEM_TEST
B2_OBJS += src/mem_test.o
B2_OBJS += src/ntv_mt.o src/ntv_mt_main.o
endif

ifdef USB
B2_OBJS += src/usb/ehci-hcd.o src/usb/usb.o src/usb/cmd_usb.o
B2_OBJS += src/usb/usb_storage.o
B2_OBJS += src/usb/usb_storage_test.o
B2_OBJS += src/usb/usb_loopback_dev.o
B2_OBJS += src/usb/vbus_detection.o
endif

ifdef I2CS
B2_OBJS += src/gpio_i2cs.o
B2_OBJS += src/mem_test.o
B2_OBJS += src/mmc/mmc.o
B2_OBJS += src/mmc/sdhci.o
B2_OBJS += src/mmc/sdhci-mt.o
endif

ifdef PCM_I2S
B2_OBJS += src/gpio_i2c.o
B2_OBJS += src/ai.o
#B2_OBJS += src/i2c_test.o
#B2_OBJS += src/pcm_test.o
#B2_OBJS += src/i2c_wm8750.o
B2_OBJS += src/i2c_mp320.o
B2_OBJS += src/i2c_tlv320aic3xxx.o
#B2_OBJS += src/i2c_nau8812.o
#B2_OBJS += src/i2c_nau8822.o
#B2_OBJS += src/i2c_analog.o
endif

ifdef SDHC
B2_OBJS += src/mmc/mmc.o
B2_OBJS += src/mmc/sdhci.o
B2_OBJS += src/mmc/sdhci-mt.o
endif

ifdef MINI_SDHC
B2_OBJS += src/mmc/mini-sdhc-ipl_boot2.o
endif

ifdef PWM
B2_OBJS += src/pwm.o
endif

#ifdef LCD
#B2_OBJS += src/lcd/lcd_st7789v_spi.o
#endif

ifdef DHCP
CFLAGS += -DCONFIG_DHCPD
B2_OBJS += src/dhcpd.o src/timer.o
endif

ifeq ($(BOOT3_SUBMODE),1)
CFLAGS += -DBOOT3_SUBMODE
endif

ifeq ($(MASS_PRODUCTION_TEST),1)
CFLAGS += -DMASS_PRODUCTION_TEST -DBOOT3_SUBMODE -DCONFIG_MADC -DRTS_CTS_TEST
ifndef PCM_I2S 
CFLAGS += -DCONFIG_PCM_I2S
B2_OBJS += src/gpio_i2c.o
B2_OBJS += src/ai.o
endif
ifndef MINI_SDHC
CFLAGS += -DCONFIG_MINI_SDHC
B2_OBJS += src/mmc/mini-sdhc-ipl_boot2.o
endif
B2_OBJS += src/madc.o src/rtscts.o
WIFI=1
WT=1
SCHED=1
endif

ifdef SCHED
CFLAGS += -DCONFIG_SCHED
B2_OBJS += src/sched.o
endif

ifdef WIFI
CFLAGS += -DCONFIG_WIFI
CFLAGS += -I./src/wifi/include -I./src/wifi/math

#build for test math library
#WIFI_OBJS = $(patsubst %, src/wifi/%, lib_test.o )

WIFI_OBJS = $(patsubst %, src/wifi/%,	init.o \
					cli_mem_ahb.o \
					cosim_main.o \
					bb.o \
					rf.o \
					rfc_comm.o \
					rfc_panther.o \
					rfc_new.o \
					rfac.o \
					rfac_rfc_patch.o \
					ip301.o \
					panther_rf.o \
					panther_rfac.o \
					complex.o \
					send_pkt.o \
					mt_driver_Panther.o \
					bbdump.o \
					beacon.o \
					mini_mlme.o \
					performance.o )

LIB_GCC_OBJS = $(patsubst %, src/wifi/libgcc/%, _addsub_df.o _df_to_si.o _div_df.o _eq_df.o _fixunsdfsi.o _fpcmp_parts_df.o \
				      _ge_df.o _gt_df.o _le_df.o _lt_df.o _mul_df.o _mul_sf.o \
				      _pack_df.o _pack_sf.o _si_to_df.o _thenan_df.o _thenan_sf.o _unpack_df.o \
				      _unpack_sf.o )

MATH_OBJS = $(patsubst %, src/wifi/math/%, e_asin.o e_log10.o e_log.o e_pow.o e_rem_pio2.o e_sqrt.o \
			k_cos.o k_rem_pio2.o k_sin.o k_tan.o s_ceil.o s_copysign.o s_cos.o s_fabs.o s_finite.o \
                        s_floor.o s_isnan.o s_scalbn.o s_sin.o s_tan.o )

ifeq ($(LE),1)
LIBS = lib/libgcc_le.a
else
LIBS = lib/libgcc_be.a
endif

B2_OBJS += $(WIFI_OBJS) $(LIB_GCC_OBJS) $(MATH_OBJS)

ifeq ($(WT),1)
CFLAGS += -DWLA_TEST
B2_OBJS += src/wifi/wtest.o
endif
endif

ifdef FATFS
B2_OBJS += src/fatfs/ff.o src/fatfs/ffsystem.o
B2_OBJS += src/fatfs/cmd_fs.o
endif

B2_OBJS += src/gdb-stub.o

# objects for boot3.img
B3_OBJS = $(B2_OBJS)

ifdef BOOT3_LCD
B3_OBJS += src/lcd/lcd_st7789v_spi.o
endif

IGNORE_FILES:=crc16.c xmodem.c
IGNORE_FILES+=LzmaDecode.c LzmaDecode.h
IGNORE_FILES+=gdb-stub.c gdb-stub.h inst.h
IGNORE_FILES+=dhry_1.c dhry_2.c dhry.h
IGNORE_FILES+=ntv_mt.c ntv_mt_main.c ntv_mt.h
IGNORE_FILES+=ehci-hcd.c usb.c cmd_usb.c
IGNORE_FILES+=usb_storage.c part.h

all: boot1_ferase boot1_debrick boot.img

include include/rules.mak
#export PATH := ../../toolchains/toolchain-mips_interaptiv_gcc-4.8-linaro_uClibc-0.9.33.2/usr/bin:$(PATH)
#export PATH := ../../toolchains/toolchain-mipsel_interaptiv_gcc-4.8-linaro_uClibc-0.9.33.2/usr/bin:$(PATH)
#export PATH := ../../toolchains/toolchain-mips_interaptiv_gcc-7.3.0_musl/bin:$(PATH)
#export PATH := ../../toolchains/toolchain-mipsel_interaptiv_gcc-7.3.0_musl/bin:$(PATH)

$(BT_LDS): $(BT_LDS:.lds=.lds.S) Makefile
	$(Q)$(XCC) -E -P $(CFLAGS) -o $@ $@.S

fw: str.bin
	$(Q)echo "unsigned char str_fw[] = \"\\" > src/str.h
	$(Q)hexdump -v -e '"\\" "x" 1/1 "%02X"' str.bin >> src/str.h
	$(Q)echo "\";" >> src/str.h
	$(Q)rm -f str.bin
	$(Q)rm -f src/hw.o

str: $(STR_OBJS) $(BT_LDS)
	$(ECHO) "\t [LD] $@"
	$(Q)$(XLD) $(LDFLAGS_STR) -T$(BT_LDS) -o $@ $(STR_OBJS)
	$(Q)$(CROSS_PREFIX)size str

boot1_ferase: $(B1_FERASE_OBJS) $(BT_LDS)
	$(ECHO) "\t [LD] $@"
	$(Q)$(XLD) $(LDFLAGS_B1_FERASE) -T$(BT_LDS) -o $@ $(B1_FERASE_OBJS)
	$(Q)$(CROSS_PREFIX)size boot1_ferase

boot1_debrick: $(B1_DEBRICK_OBJS) $(BT_LDS)
	$(ECHO) "\t [LD] $@"
	$(Q)$(XLD) $(LDFLAGS_B1_DEBRICK) -T$(BT_LDS) -o $@ $(B1_DEBRICK_OBJS)
	$(Q)$(CROSS_PREFIX)size boot1_debrick

boot1: $(B1_OBJS) $(BT_LDS)
	$(ECHO) "\t [LD] $@"
	$(Q)$(XLD) $(LDFLAGS_B1) -T$(BT_LDS) -o $@ $(B1_OBJS)
	$(Q)$(CROSS_PREFIX)size boot1

boot1_crc16:
	-$(Q)gcc $(BOOT1_24K) -o tools/boot1_crc16 tools/boot1_crc16.c

boot1.img: boot1.bin boot1_crc16
	$(ECHO) "\t [CRC16] boot1"
	$(Q)cp boot1.bin boot1.img
	-$(Q)./tools/boot1_crc16
	$(Q)rm -f ./boot1_crc16
	$(Q)cp -f boot1.img sboot1.img
	-$(Q)../../tools/msign/msign -M -s ../../tools/msign/key -i boot1.img -b 2048 -o sboot1.img
	
boot2: fw $(B2_OBJS) $(BT_LDS)
	$(ECHO) "\t [LD] $@"
	$(Q)$(XLD) $(LDFLAGS_B2) -T$(BT_LDS) -o $@ $(B2_OBJS) $(LIBS)
	$(Q)$(CROSS_PREFIX)size boot2

boot.bin: boot1.bin boot1.img $(B2_ZIP)
	$(Q)tr "\000" "\377" < /dev/zero | dd of=$@ ibs=1 count=128K &> /dev/null
	$(Q)dd if=boot1.img of=$@ obs=1 conv=notrunc &> /dev/null
	$(Q)dd if=$(B2_ZIP) of=$@ obs=1 seek=32K conv=notrunc &> /dev/null
	$(ECHO) boot1.bin size: \(budget: 16380 bytes / 24572 bytes \(with IPL B1 24K option\)\)
	$(Q)stat -c%s boot1.bin
	$(ECHO) $(B2_ZIP) size: \(budget: 98304 bytes\)
	$(Q)stat -c%s $(B2_ZIP)

boot.img: boot.bin
	$(ECHO) "\t[IMG] $@"
	$(Q)if [ $(shell stat -c%s $<) -gt 131072 ]; then echo -e "error: $@ over 128KB"; rm -f $@; else cp $< $@; fi
	$(Q)cp -f boot.img sboot.img
	-$(Q)../../tools/msign/msign -M -s ../../tools/msign/key -i boot.img -b 1024 -o sboot.img
	$(Q)tr "\000" "\377" < /dev/zero | dd of=erased.img ibs=1 count=8K &> /dev/null
	$(Q)dd if=erased.img of=sboot.img obs=1 seek=24K conv=notrunc &> /dev/null
	$(Q)rm -f erased.img
	$(Q)dd if=sboot1.img of=sboot.img conv=notrunc &> /dev/null

boot3: fw $(B3_OBJS) $(BT_LDS)
	$(ECHO) "\t [LD] $@"
	$(Q)$(XLD) $(LDFLAGS_B3) -T$(BT_LDS) -o boot3 $(B3_OBJS) $(LIBS)
	$(Q)$(CROSS_PREFIX)size boot3

# build boot3.img for boot2 testing(use CAPP header with command "go buf")
boot3.img: $(B3_ZIP)
	$(ECHO) boot3.bin.lzma size:
	$(Q)stat -c%s $(B3_ZIP)
	$(Q)$(XCC) -o ih.o -c include/ih_temp.c -DIH_HLEN=$(HEAD_LEN) -DIH_TIME=$(shell date -d 'now +8 hour' +'%s') -DIH_RUN=$(B3_TEXT)\
		 -DIH_LOAD=$(B3_TEXT) -DIH_SIZE=$(shell expr $(shell expr $(shell expr $(shell stat -c%s $(B3_ZIP)) + 3) / 4) \* 4) -DIH_MID=0x1234 -DIH_VER=0x5678 -DIH_CHKSUM=0
	$(Q)$(CROSS_PREFIX)objcopy -j .data -O binary ih.o ih.bin
	$(Q)tr "\000" "\377" < /dev/zero | dd of=$@ ibs=1 count=$(shell expr $(shell expr $(shell expr $(shell stat -c%s $(B3_ZIP)) + 2047 + 48) / 2048) \* 2048) &> /dev/null
	$(Q)dd if=ih.bin of=$@ obs=1 conv=notrunc &> /dev/null
	$(Q)dd if=$(B3_ZIP) of=$@ obs=1 seek=48 conv=notrunc &> /dev/null
	$(Q)rm -f ih.o ih.bin
	-$(Q)../../tools/msign/msign -M -s ../../tools/msign/key -b 256 -i boot3.img -o sboot3_nor.img
	-$(Q)../../tools/msign/msign -M -s ../../tools/msign/key -b 2048 -i boot3.img -o sboot3_nand_128k.img

boot.txt: boot.img
	rm -f boot.txt sboot.txt
	-od -An -vtx1 -w1 boot.img | cut -c2- > boot.txt
	-od -An -vtx1 -w1 sboot.img | cut -c2- > sboot.txt

app.img:
	@ln -sf ../capp.img app.img

sflash.img: boot.img app.img
	$(ECHO) "\t[IMG] $@"
	$(Q)tr "\000" "\377" < /dev/zero | dd of=$@ ibs=1 count=128K &> /dev/null
	$(Q)dd if=$< of=$@ conv=notrunc &> /dev/null
	$(Q)dd if=app.img of=$@ obs=1 seek=128K conv=notrunc &> /dev/null

run: boot2
	$(Q)$(XDB) -x gdb_scripts/run_boot2.gdb
	
load: boot2
	$(Q)$(XDB) -x gdb_scripts/load_boot2.gdb

clean:
	$(ECHO) "\t[CLEAN]"
	$(Q)rm -f boot1_ferase boot1_debrick boot1 boot2 boot3 str
	$(Q)find -regex '.*\.lzma\|.*\.[osd]\|.*\.bin\|.*\.img\|.*\.lds' -print | xargs rm -f
	$(Q)rm -f *.map *.dis *.cdb *.rc
	$(Q)rm -f *.lzo
	$(Q)rm -f *.tmp
	$(Q)rm -f src/str.h

debrick:
	make
	mips-openwrt-linux-gdb -x debrick/reset.gdb
	sleep 1
	mips-openwrt-linux-gdb -x debrick/boot1_debrick.gdb
	sleep 1
	mips-openwrt-linux-gdb -x debrick/boot2_debrick.gdb
	#sleep 1
	#mips-openwrt-linux-gdb -x gdb_scripts/upload_boot_img.gdb
	#init flash 2
	#fe 0 20000 81000000
	#fw 0 20000 81000000

debrick_le:
	make LE=1
	mips-openwrt-linux-gdb -x debrick/reset.gdb
	sleep 1
	mips-openwrt-linux-gdb -x debrick/boot1_debrick.gdb
	sleep 1
	mips-openwrt-linux-gdb -x debrick/boot2_debrick.gdb

sdio_program:
	dd if=boot.img of=/dev/sde bs=512 seek=8

beautify:
	echo $(IGNORE_FILES)
	find -name \*.[ch] $(foreach f, $(IGNORE_FILES), -a ! -name $f) |xargs indent -kr -nut -i4 -sob -l80 -ss -bs -bl -bls -bli0 -cli4 -cbi0 -nce
	find -name \*.[ch] $(foreach f, $(IGNORE_FILES), -a ! -name $f) |xargs indent -kr -nut -i4 -sob -l80 -ss -bs -bl -bls -bli0 -cli4 -cbi0 -nce

.PHONY: all clean beautify debrick fw
