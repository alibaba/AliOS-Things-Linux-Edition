DESCRIPTION = "Openwrt backported WLAN drivers."
SECTION = "Openwrt drivers."
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "file://backports-2017-11-01.tar.xz \
file://.config \
file://000-fix_kconfig.patch \
file://001-fix_build.patch \
file://002-change_allconfig.patch \
file://003-remove_bogus_modparams.patch \
file://004-kconfig_backport_fix.patch \
file://005-revert-devcoredump.patch \
file://006-fix-genl-multicast.patch \
file://007-fix-linux-verification-h.patch \
file://008-fix-genl-family-id.patch \
file://010-disable_rfkill.patch \
file://012-kernel_build_check.patch \
file://015-ipw200-mtu.patch \
file://030-rt2x00_options.patch \
file://040-brcmutil_option.patch \
file://050-lib80211_option.patch \
file://060-no_local_ssb_bcma.patch \
file://070-ath_common_config.patch \
file://080-ath10k_thermal_config.patch \
file://100-remove-cryptoapi-dependencies.patch \
file://110-mac80211_keep_keys_on_stop_ap.patch \
file://120-cfg80211_allow_perm_addr_change.patch \
file://130-disable-fils.patch \
file://131-Revert-mac80211-aes-cmac-switch-to-shash-CMAC-driver.patch \
file://132-mac80211-remove-cmac-dependency.patch \
file://140-tweak-TSQ-setting.patch \
file://150-disable_addr_notifier.patch \
file://201-ath5k-WAR-for-AR71xx-PCI-bug.patch \
file://210-ap_scan.patch \
file://300-ath9k_hw-reset-AHB-WMAC-interface-on-AR91xx.patch \
file://301-ath9k_hw-issue-external-reset-for-QCA955x.patch \
file://302-ath9k_hw-set-spectral-scan-enable-bit-on-trigger-for.patch \
file://303-ath9k-don-t-run-periodic-and-nf-calibation-at-the-sa.patch \
file://304-ath9k-force-rx_clear-when-disabling-rx.patch \
file://305-ath9k-limit-retries-for-powersave-response-frames.patch \
file://306-Revert-ath9k-interpret-requested-txpower-in-EIRP-dom.patch \
file://307-mac80211-add-hdrlen-to-ieee80211_tx_data.patch \
file://308-mac80211-add-NEED_ALIGNED4_SKBS-hw-flag.patch \
file://309-mac80211-minstrel-Enable-STBC-and-LDPC-for-VHT-Rates.patch \
file://310-ath9k-fix-moredata-bit-in-PS-buffered-frame-release.patch \
file://311-ath9k-clear-potentially-stale-EOSP-status-bit-in-int.patch \
file://312-ath9k-report-tx-status-on-EOSP.patch \
file://313-ath9k-fix-block-ack-window-tracking-issues.patch \
file://314-ath9k_hw-fix-channel-maximum-power-level-test.patch \
file://315-ath9k-adjust-tx-power-reduction-for-US-regulatory-do.patch \
file://316-ath9k-fix-more-data-flag-for-buffered-multicast-pack.patch \
file://317-Revert-ath10k-disable-wake_tx_queue-for-older-device.patch \
file://318-ath10k-fix-build-errors-with-CONFIG_PM.patch \
file://319-ath10k-fix-recent-bandwidth-conversion-bug.patch \
file://320-mac80211-properly-free-requested-but-not-started-TX-.patch \
file://400-ath_move_debug_code.patch \
file://401-ath9k_blink_default.patch \
file://402-ath_regd_optional.patch \
file://403-world_regd_fixup.patch \
file://404-regd_no_assoc_hints.patch \
file://405-ath_regd_us.patch \
file://406-ath_relax_default_regd.patch \
file://410-ath9k_allow_adhoc_and_ap.patch \
file://411-ath5k_allow_adhoc_and_ap.patch \
file://420-ath5k_disable_fast_cc.patch \
file://430-add_ath5k_platform.patch \
file://431-add_platform_eeprom_support_to_ath5k.patch \
file://432-ath5k_add_pciids.patch \
file://440-ath5k_channel_bw_debugfs.patch \
file://500-ath9k_eeprom_debugfs.patch \
file://501-ath9k_ahb_init.patch \
file://510-ath9k_intr_mitigation_tweak.patch \
file://511-ath9k_reduce_rxbuf.patch \
file://512-ath9k_channelbw_debugfs.patch \
file://513-ath9k_add_pci_ids.patch \
file://522-mac80211_configure_antenna_gain.patch \
file://530-ath9k_extra_leds.patch \
file://531-ath9k_extra_platform_leds.patch \
file://540-ath9k_reduce_ani_interval.patch \
file://542-ath9k_debugfs_diag.patch \
file://543-ath9k_entropy_from_adc.patch \
file://544-ath9k-ar933x-usb-hang-workaround.patch \
file://545-ath9k_ani_ws_detect.patch \
file://547-ath9k_led_defstate_fix.patch \
file://548-ath9k_enable_gpio_chip.patch \
file://549-ath9k_enable_gpio_buttons.patch \
file://550-ath9k-disable-bands-via-dt.patch \
file://551-ath9k_ubnt_uap_plus_hsr.patch \
file://600-01-rt2x00-allow-to-build-rt2800soc-module-for-RT3883.patch \
file://600-02-rt2x00-rt2800lib-enable-support-for-RT3883.patch \
file://600-03-rt2x00-rt2800lib-add-rf_vals-for-RF3853.patch \
file://600-04-rt2x00-rt2800lib-enable-VCO-calibration-for-RF3853.patch \
file://600-05-rt2x00-rt2800lib-add-channel-configuration-function-.patch \
file://600-06-rt2x00-rt2800lib-enable-RF3853-support.patch \
file://600-07-rt2x00-rt2800lib-add-MAC-register-initialization-for.patch \
file://600-08-rt2x00-rt2800soc-fix-rt2800soc_disable_radio-for-RT3.patch \
file://600-09-rt2x00-rt2800lib-add-BBP-register-initialization-for.patch \
file://600-10-rt2x00-rt2800lib-add-RFCSR-initialization-for-RT3883.patch \
file://600-11-rt2x00-rt2800lib-use-the-extended-EEPROM-map-for-RT3.patch \
file://600-12-rt2x00-rt2800lib-force-rf-type-to-RF3853-on-RT3883.patch \
file://600-13-rt2x00-rt2800lib-add-channel-configuration-code-for-.patch \
file://600-14-rt2x00-rt2800lib-fix-txpower_to_dev-function-for-RT3.patch \
file://600-15-rt2x00-rt2800lib-use-correct-txpower-calculation-fun.patch \
file://600-16-rt2x00-rt2800lib-hardcode-txmixer-gain-values-to-zer.patch \
file://600-17-rt2x00-rt2800lib-use-correct-RT-XWI-size-for-RT3883.patch \
file://600-18-rt2x00-rt2800lib-fix-antenna-configuration-for-RT388.patch \
file://600-19-rt2x00-rt2800lib-fix-LNA-gain-configuration-for-RT38.patch \
file://600-20-rt2x00-rt2800lib-fix-VGC-setup-for-RT3883.patch \
file://600-21-rt2x00-rt2800lib-fix-EEPROM-LNA-validation-for-RT388.patch \
file://600-22-rt2x00-rt2800lib-fix-txpower-compensation-for-RT3883.patch \
file://600-23-rt2x00-rt2800mmio-add-a-workaround-for-spurious-TX_F.patch \
file://601-rt2x00-introduce-rt2x00_platform_h.patch \
file://602-rt2x00-introduce-rt2x00eeprom.patch \
file://603-rt2x00-of_load_eeprom_filename.patch \
file://604-rt2x00-load-eeprom-on-SoC-from-a-mtd-device-defines-.patch \
file://606-rt2x00-allow_disabling_bands_through_platform_data.patch \
file://607-rt2x00-add_platform_data_mac_addr.patch \
file://608-rt2x00-allow_disabling_bands_through_dts.patch \
file://609-rt2x00-make-wmac-loadable-via-OF-on-rt288x-305x-SoC.patch \
file://610-rt2x00-change-led-polarity-from-OF.patch \
file://611-rt2x00-add-AP+STA-support.patch \
file://650-rt2x00-add-support-for-external-PA-on-MT7620.patch \
file://651-rt2x00-remove-unneccesary-code.patch \
file://653-0001-rtl8xxxu-Accept-firmware-signature-0x88e0.patch \
file://653-0002-rtl8xxxu-Add-initial-code-to-detect-8188eu-devices.patch \
file://653-0003-rtl8xxxu-Add-initial-code-to-parse-8188eu-efuse.patch \
file://653-0004-rtl8xxxu-Detect-8188eu-parts-correctly.patch \
file://653-0005-rtl8xxxu-First-stab-at-rtl8188e_power_on.patch \
file://653-0006-rtl8xxxu-Add-rtl8188e_disabled_to_emu.patch \
file://653-0007-rtl8xxxu-8188e-Enable-scheduler.patch \
file://653-0008-rtl8xxxu-Add-rtl8188e_usb_quirk-for-enabling-MAC-TX-.patch \
file://653-0009-rtl8xxxu-8188e-add-REG_TXDMA_OFFSET_CHK-quirk.patch \
file://653-0010-rtl8xxxu-Add-reserved-page-init-parameters-for-8188e.patch \
file://653-0011-rtl8xxxu-Correct-TX_TOTAL_PAGE_NUM-for-8188eu.patch \
file://653-0012-rtl8xxxu-Add-trxff_boundary-for-8188e.patch \
file://653-0013-rtl8xxxu-8188eu-specify-firmware-block-size-and-set-.patch \
file://653-0014-rtl8xxxu-Add-8188e-mac-init-table.patch \
file://653-0015-rtl8xxxu-Implement-rtl8188eu_init_phy_bb.patch \
file://653-0016-rtl8xxxu-Implement-rtl8188eu_init_phy_rf.patch \
file://653-0017-rtl8xxxu-Use-auto-LLT-init-for-8188e.patch \
file://653-0018-rtl8xxxu-Do-not-set-REG_FPGA0_TX_INFO-on-8188eu.patch \
file://653-0019-rtl8xxxu-Do-not-mess-with-REG_FPGA0_XA_RF_INT_OE-eit.patch \
file://653-0020-rtl8xxxu-Set-transfer-page-size-for-8188eu.patch \
file://653-0021-rtl8xxxu-Enable-TX-report-timer-on-8188eu.patch \
file://653-0022-rtl8xxxu-Setup-interrupts-for-8188eu.patch \
file://653-0023-rtl8xxxu-Use-rxdesc16-and-32-byte-tx-descriptors-for.patch \
file://653-0024-rtl8xxxu-8188eu-use-same-ADDA-on-parameters-as-8723a.patch \
file://653-0025-rtl8xxxu-Add-PHY-IQ-calibration-code-for-8188eu.patch \
file://653-0026-rtl8xxxu-8188eu-uses-the-gen2-thermal-meter.patch \
file://653-0027-rtl8xxxu-Set-REG_USB_HRPWM-to-0-for-8188eu.patch \
file://653-0028-rtl8xxxu-Implement-rtl8188eu_config_channel.patch \
file://653-0029-rtl8xxxu-Use-gen2-H2C-commands-for-8188eu.patch \
file://653-0030-rtl8xxxu-Initialize-GPIO-settings-for-8188eu.patch \
file://653-0031-rtl8xxxu-Add-simple-rtl8188eu_rf_on-routine.patch \
file://653-0032-rtl8xxxu-Implement-rtl8188e_disable_rf.patch \
file://653-0033-rtl8xxxu-Update-8188e-efuse-definition-for-power-val.patch \
file://653-0034-rtl8xxxu-Implement-rtl8188e_set_tx_power.patch \
file://653-0035-rtl8xxxu-Implement-rtl8xxxu_fill_txdesc_v3-for-8188e.patch \
file://653-0036-rtl8xxxu-Add-some-8188eu-registers-and-update-CCK0_A.patch \
file://653-0037-rtl8xxxu-Improve-register-description-for-REG_FPGA1_.patch \
file://653-0038-rtl8xxxu-properly-detect-RTL8188EU-devices.patch \
file://653-0039-rtl8xxxu-Implement-8188eu-specific-8051-reset-functi.patch \
file://653-0040-rtl8xxxu-Disable-packet-DMA-aggregation-on-8188eu.patch \
file://653-0041-rtl8xxxu-8188eu-set-REG_OFDM0_XA_AGC_CORE1-to-match-.patch \
file://653-0042-rtl8xxxu-Fix-rtl8188eu-connection-fail.patch \
file://653-0043-rtl8xxxu-Do-not-set-auto-rate-fallback-on-8188eu.patch \
file://653-0044-rtl8xxxu-Enable-8188eu-driver.patch \
file://653-0045-rtl8xxxu-Add-rtl8188etv-to-USB-device-list.patch \
file://653-0046-rtl8xxxu-Add-sitecom-dongle-to-USB-device-list.patch \
file://653-0047-rtl8xxxu-Implement-rtl8188eu_active_to_emu.patch \
file://653-0048-rtl8xxxu-Implement-rtl8188eu_power_off.patch \
file://653-0049-rtl8xxxu-Add-rtl8188eu-USB-ID-for-D-Link-USB-GO-N150.patch \
file://653-0050-rtl8xxxu-Clear-SYS_FUNC_UPLL-during-power-up-on-8188.patch \
file://653-0051-rtl8xxxu-Early-enable-of-WEP-TKIP-security-on-8188eu.patch \
file://653-0052-rtl8xxxu-Correct-power-down-sequence-for-8188eu.patch \
file://653-0053-rtl8xxxu-Reset-8188eu-REG_GPIO_MUXCFG-on-power-off.patch \
file://653-0054-rtl8xxxu-Handle-devices-with-a-smaller-LLT-buffer.patch \
file://653-0055-rtl8xxxu-Fix-reloading-of-driver-for-8188eu-devices.patch \
file://653-0056-rtl8xxxu-Make-sure-to-enable-OFDM-paths-for-8188eu-i.patch \
file://653-0057-rtl8xxxu-Add-rpt_sel-entry-to-struct-rtl8xxxu_rxdesc.patch \
file://700-mwl8k-missing-pci-id-for-WNR854T.patch \
file://801-libertas-configure-sysfs-links.patch \
file://802-libertas-set-wireless-macaddr.patch \
file://810-b43-gpio-mask-module-option.patch \
file://811-b43_no_pio.patch \
file://812-b43-add-antenna-control.patch \
file://813-b43-reduce-number-of-RX-slots.patch \
file://814-b43-only-use-gpio-0-1-for-led.patch \
file://815-b43-always-take-overlapping-devs.patch \
file://850-brcmsmac-remove-extra-regulation-restriction.patch \
file://860-brcmfmac-register-wiphy-s-during-module_init.patch \
file://861-brcmfmac-workaround-bug-with-some-inconsistent-BSSes.patch \
file://862-brcmfmac-Disable-power-management.patch \
file://863-brcmfmac-add-in-driver-tables-with-country-codes.patch \
file://864-brcmfmac-do-not-use-internal-roaming-engine-by-default.patch \
file://921-ath10k_init_devices_synchronously.patch \
file://930-ath10k_add_tpt_led_trigger.patch \
file://936-ath10k-fix-otp-failure-result.patch \
file://940-mwl8k_init_devices_synchronously.patch \
file://960-0010-ath10k-limit-htt-rx-ring-size.patch \
file://960-0011-ath10k-limit-pci-buffer-size.patch \
file://961-disable-kconf-target-build.patch \
file://962-fix-missing-seq-command.patch \
"
S = "${WORKDIR}/backports-2017-11-01"

DEPENDS = "kconf-native"

inherit module

PKG_VERSION = "2017-11-01"
PKG_RELEASE = "1"
REVISION = "r5671-c5ca1c9"



do_configure() {
	rm -rf \
		${S}/include/linux/ssb \
		${S}/include/linux/bcma \
		${S}/include/net/bluetooth

	rm -f \
		${S}/include/linux/cordic.h \
		${S}/include/linux/crc8.h \
		${S}/include/linux/eeprom_93cx6.h \
		${S}/include/linux/wl12xx.h \
		${S}/include/linux/spi/libertas_spi.h \
		${S}/include/net/ieee80211.h \
		${S}/backport-include/linux/bcm47xx_nvram.h

	echo 'compat-wireless-${PKG_VERSION}-${PKG_RELEASE}-${REVISION}' > ${S}/compat_version

	cmp ${S}/include/linux/ath9k_platform.h ${STAGING_KERNEL_DIR}/include/linux/ath9k_platform.h
	cmp ${S}/include/linux/ath5k_platform.h ${STAGING_KERNEL_DIR}/include/linux/ath5k_platform.h
	cmp ${S}/include/linux/rt2x00_platform.h ${STAGING_KERNEL_DIR}/include/linux/rt2x00_platform.h

	cp ${WORKDIR}/.config ${S}
	cp `which mconf` ${S}/kconf
	cp `which conf` ${S}/kconf
}

LDFLAGS = ""

EXTRA_OEMAKE="-C "${S}" \
	EXTRA_CFLAGS="-I${S}/include" \
	KLIB_BUILD="${STAGING_KERNEL_BUILDDIR}" \
	MODPROBE=true \
	KLIB=${STAGING_KERNEL_BUILDDIR} \
	KERNEL_SUBLEVEL=73 \
	KBUILD_LDFLAGS_MODULE_PREREQ="

do_install_append() {
	mkdir -p \
		${STAGING_KERNEL_DIR}/usr/include/mac80211 \
		${STAGING_KERNEL_DIR}/usr/include/mac80211-backport \
		${STAGING_KERNEL_DIR}/usr/include/mac80211/ath \
		${STAGING_KERNEL_DIR}/usr/include/net/mac80211
	cp -r ${S}/net/mac80211/*.h ${S}/include/* ${STAGING_KERNEL_DIR}/usr/include/mac80211/
	cp -r ${S}/backport-include/* ${STAGING_KERNEL_DIR}/usr/include/mac80211-backport/
	cp -r ${S}/net/mac80211/rate.h ${STAGING_KERNEL_DIR}/usr/include/net/mac80211/
	cp -r ${S}/drivers/net/wireless/ath/*.h ${STAGING_KERNEL_DIR}/usr/include/mac80211/ath/
	rm -f ${STAGING_KERNEL_DIR}/usr/include/mac80211-backport/linux/module.h
}
