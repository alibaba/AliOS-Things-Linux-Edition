# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

inherit native deploy

DESCRIPTION = "Rockchip binary tools"

LICENSE = "LICENSE.rockchip"
LIC_FILES_CHKSUM = "file://${RK_BINARY_LICENSE};md5=5fd70190c5ed39734baceada8ecced26"

SRC_URI = " \
	git://github.com/rockchip-linux/rkbin.git;branch=29mirror \
	file://afptool \
	file://rkImageMaker \
"

SRCREV = "b789f812b52931534b7b6cf762428f94e3942bec"
S = "${WORKDIR}/git"

# The pre-built tools have different link loader, don't change them.
SSTATEPOSTUNPACKFUNCS_remove += "uninative_changeinterp"

do_install () {
	install -d ${D}/${bindir}
	install -m 0755 "${S}/tools/boot_merger" ${D}/${bindir}
	install -m 0755 "${S}/tools/trust_merger" ${D}/${bindir}
	install -m 0755 "${S}/tools/firmwareMerger" ${D}/${bindir}

	install -m 0755 "${S}/tools/kernelimage" ${D}/${bindir}
	install -m 0755 "${S}/tools/loaderimage" ${D}/${bindir}

	install -m 0755 "${S}/tools/mkkrnlimg" ${D}/${bindir}
	install -m 0755 "${S}/tools/resource_tool" ${D}/${bindir}

	install -m 0755 "${S}/tools/upgrade_tool" ${D}/${bindir}

	install -m 0755 "${WORKDIR}/afptool" ${D}/${bindir}
	install -m 0755 "${WORKDIR}/rkImageMaker" ${D}/${bindir}
}
