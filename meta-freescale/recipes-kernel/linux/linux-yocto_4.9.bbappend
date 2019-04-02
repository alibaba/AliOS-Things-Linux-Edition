FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}/${BPN}_${LINUX_VERSION}:"

SRC_URI_append = " \
                 file://0001-port-patches-from-linux-imx-4.9.11-to-linux-yocto-4..patch \
                 ${@bb.utils.contains('LINUX_VERSION', '4.9.155', 'file://0002-Fix-patch-conflicts.patch', '', d)} \
                 file://defconfig \
                 "

COMPATIBLE_MACHINE += "|(mx6|mx7)"
