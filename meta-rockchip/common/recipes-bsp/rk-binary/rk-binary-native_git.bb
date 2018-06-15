# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

inherit native deploy

DESCRIPTION = "Rockchip binary tools"

LICENSE = "LICENSE.rockchip"
LIC_FILES_CHKSUM = "file://${RK_BINARY_LICENSE};md5=5fd70190c5ed39734baceada8ecced26"

SRC_URI = "git://github.com/rockchip-linux/rkbin.git;branch=29mirror"
SRCREV = "2a2ab345a98c7f41cd3fcb09f820828326fca596"
S = "${WORKDIR}/git"

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
}
