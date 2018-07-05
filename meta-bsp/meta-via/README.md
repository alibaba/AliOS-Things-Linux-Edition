# meta-via

Yocto BSP layer for VIA i.MX6 serial boards
[Small Form Factor PCs](https://www.viatech.com/en/systems/small-form-factor-pcs/)
[Industrial Fanless PCs](https://www.viatech.com/en/systems/industrial-fanless-pcs/)

# Dependencies

This layer depends on:

* alios-bsp-sdk
* meta-freescale

# Configure Environment

1. Add below layers to build/conf/bblayer.conf
   * ${TOPDIR}/../meta-bsp/meta-via/meta-freescale
   * ${TOPDIR}/../meta-bsp/meta-via/meta-via-arm

2. Modify build/conf/local.conf file
   * Remove/Mark line:
     * IMAGE_FSTYPES += "iso tar.bz2"
     * MACHINE ??= "intel-corei7-64"
     * DISTRO ?= "AliOS"

   * Modify line:
     * DISTRO ?= "AliOS-via"

   * Add Line:
     * ACCEPT_FSL_EULA = "1"

   * Set machine name:
     * ARTIGO A820: MACHINE ??= "imx6artigoa820"

# Quick Start

1. source oe-init-build-env
2. Follow section "Configure Environment"
3. bitbake [IMAGE]
   * core-image-via (included basic tools)
     * ppp, python, minicom, pciutils, parted, dosfstools
     * e2fsprogs, usbutils, alsa-utils, imx-rs485-tool
   * core-image-base
   * core-image-minimal

# Reflash File System
* Build SD card image
  * goto path /build/tmp/deploy/images/[MACHINE]/[IMAGE]--sd_installer
  * run command: sudo ./mk_sd_installer.sh /dev/sdx --yocto
    (sdx, node of sd device, exp /dev/sdb)
  * switch boot device to u-SD (open the cover on the bottom of the system)
  * insert SD card and power up the system
  * login account: root
* Build SD card installer for reflash file system to eMMC
  * IMAGE must using core-image-via
  * goto path /build/tmp/deploy/images/[MACHINE]/[IMAGE]--sd_installer
  * run command: sudo ./mk_sd_installer.sh /dev/sdx
  * wait message: Done! The SD card can be removed now!
  * switch boot device to u-SD (open the cover on the bottom of the system)
  * insert SD card and power up the system
  * login account: root
  * cd /mnt
  * mkdir mmcblk1p2
  * mount /dev/mmcblk1p2 mmcblk1p2/
  * cd mmcblk1p2
  * run command: ./reflash_system.sh
  * run command: shutdown -P 0 (after Finish. All successed.)
  * remove SD card
  * switch boot device to SPI (open the cover on the bottom of the system)
  * power up the system
