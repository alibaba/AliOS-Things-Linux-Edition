# Copyright (C) 2014 Freescale Semiconductor
# Released under the MIT license (see COPING. MIT for the terms)

DESCRIPTION = "VIA package group - extra"
LICENSE = "CLOSED"

inherit packagegroup
RDEPENDS_${PN} = " \
	ppp \
	parted \
	python \
	minicom \
	pciutils \
	usbutils \
    e2fsprogs \
	dosfstools \
	alsa-utils \
	via-extra \
	imx-rs485-tool \
	"
