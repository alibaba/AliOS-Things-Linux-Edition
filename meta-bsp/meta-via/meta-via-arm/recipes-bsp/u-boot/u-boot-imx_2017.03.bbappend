# Copyright (C) 2014, 2016 VIA Technologies, Inc.

DESCRIPTION = "Bootloader for i.MX platforms."

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://uboot-imx.patch"

COMPATIBLE_MACHINE = "(mx6)"
