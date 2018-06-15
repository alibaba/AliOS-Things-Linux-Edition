# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require linux-rockchip.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/files/${PN}_${LINUX_VERSION}:"
KBUILD_DEFCONFIG = "defconfig"

SRC_URI += " \
	file://0001-UPSTREAM-clk-move-the-common-clock-s-to_clk_-_hw-mac.patch \
	file://0002-UPSTREAM-clk-add-flag-for-clocks-that-need-to-be-ena.patch \
	file://0003-UPSTREAM-ARM-8478-2-arm-arm64-add-arm-smccc.patch \
	file://0004-UPSTREAM-serial-core-remove-baud_rates-when-serial-c.patch \
	file://0005-UPSTREAM-pwm-Introduce-the-pwm_args-concept.patch \
	file://0006-UPSTREAM-pwm-Add-missing-newline.patch \
	file://0007-UPSTREAM-pwm-Use-kcalloc-instead-of-kzalloc.patch \
	file://0008-UPSTREAM-pwm-Fix-pwm_apply_args-call-sites.patch \
	file://0009-UPSTREAM-pwm-Get-rid-of-pwm-lock.patch \
	file://0010-UPSTREAM-pwm-Keep-PWM-state-in-sync-with-hardware-st.patch \
	file://0011-UPSTREAM-pwm-Introduce-the-pwm_state-concept.patch \
	file://0012-UPSTREAM-pwm-Move-the-enabled-disabled-info-into-pwm.patch \
	file://0013-UPSTREAM-pwm-Add-hardware-readout-infrastructure.patch \
	file://0014-UPSTREAM-pwm-Add-core-infrastructure-to-allow-atomic.patch \
	file://0015-UPSTREAM-pwm-Switch-to-the-atomic-API.patch \
	file://0016-Sync-BSP-kernel-s-clk.patch \
	file://0017-Sync-BSP-kernel-s-grf-rk_vendor_storage.patch \
	file://0018-Sync-BSP-kernel-s-rk_nand.patch \
	file://0019-Sync-BSP-kernel-s-rockchip-timer.patch \
	file://0020-Sync-BSP-kernel-s-rockchip-iommu.patch \
	file://0021-Sync-BSP-kernel-s-pwm-rockchip.patch \
	file://0022-Sync-BSP-kernel-s-pinctrl-rockchip.patch \
	file://0023-Sync-BSP-kernel-s-rockchip-io-domain.patch \
	file://0024-arm64-Support-building-rockchip-style-images.patch \
	file://0025-arm64-configs-Add-rk3308_linux_defconfig.patch \
	file://0026-arm64-dts-Add-rk3308-evb-dmic-pdm-v11.dts.patch \
	file://defconfig \
"

COMPATIBLE_MACHINE = ".*(rk3308).*"
