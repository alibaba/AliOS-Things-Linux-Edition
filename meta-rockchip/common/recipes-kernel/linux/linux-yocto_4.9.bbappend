# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require linux-rockchip.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/files/${PN}_${LINUX_VERSION}:"
KBUILD_DEFCONFIG = "defconfig"

SRC_URI += " \
	file://0001-UPSTREAM-ASoC-soc-core-snd_soc_get_dai_name-become-n.patch \
	file://0002-Sync-BSP-kernel-s-clk-for-rk3308.patch \
	file://0003-Sync-BSP-kernel-s-grf-rk_vendor_storage.patch \
	file://0004-Sync-BSP-kernel-s-rk_nand.patch \
	file://0005-Sync-BSP-kernel-s-rockchip-iommu.patch \
	file://0006-Sync-BSP-kernel-s-pwm-rockchip.patch \
	file://0007-Sync-BSP-kernel-s-pinctrl-rockchip.patch \
	file://0008-Sync-BSP-kernel-s-rockchip-io-domain.patch \
	file://0009-Sync-BSP-kernel-s-sound.patch \
	file://0010-Sync-BSP-kernel-s-mmc.patch \
	file://0011-wireless-brcmfmac-Add-support-for-brcm43455.patch \
	file://0012-arm64-Support-building-rockchip-style-images.patch \
	file://0013-arm64-configs-Add-rk3308_linux_defconfig.patch \
	file://0014-arm64-dts-Add-rk3308-evb-dmic-pdm-v11.dts.patch \
	file://0015-Sync-BSP-kernel-s-pdm-mic.patch \
	file://defconfig \
"

COMPATIBLE_MACHINE = ".*(rk3308).*"
