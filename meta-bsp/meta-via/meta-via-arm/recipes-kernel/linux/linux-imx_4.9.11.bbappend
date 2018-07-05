# Copyright (C) 2014-2018 VIA Technologies, Inc.

SUMMARY = "Linux Kernel provided by VIA Technologies, Inc."
DESCRIPTION = "Linux Kernel provided by VIA Technologies, Inc."

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
	file://via_defconfig \
	file://linux-imx.patch \
	"

SRC_URI_append_imx6artigoa820 = " \
	file://imx6qdl-artigoa820.dtsi \
	file://imx6dl-artigoa820.dts \
	file://imx6q-artigoa820.dts \
	file://imx6qp-artigoa820.dts \
	"

DTS_DIR = "${S}/arch/arm/boot/dts/"

addtask copy_defconfig after do_unpack before do_configure
do_copy_defconfig() {
	mkdir -p ${B}
	cp ${WORKDIR}/via_defconfig ${B}/.config
	cp ${WORKDIR}/via_defconfig ${B}/../defconfig
}

do_copy_defconfig_append_imx6artigoa820() {
	cp ${WORKDIR}/imx6qdl-artigoa820.dtsi ${DTS_DIR}
	cp ${WORKDIR}/imx6dl-artigoa820.dts ${DTS_DIR}
	cp ${WORKDIR}/imx6q-artigoa820.dts ${DTS_DIR}
	cp ${WORKDIR}/imx6qp-artigoa820.dts ${DTS_DIR}
}
