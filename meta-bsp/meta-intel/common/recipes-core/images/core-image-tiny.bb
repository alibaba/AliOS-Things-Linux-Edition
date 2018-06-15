SUMMARY = "A tiny image just capable of allowing a device to boot from RAM, \
this image recipe generates an image file which rather boots from initrd than \
from storage, it achieves this by using wic to pick up the artifacts generated \
by the core-image-tiny-initramfs image"

# The actual rootfs/initrd will be the one from core-image-tiny-initramfs, so
# we reset IMAGE_INSTALL to avoid building other things that will be pointless
IMAGE_INSTALL = ""

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

IMAGE_LINGUAS = " "

LICENSE = "MIT"

IMAGE_ROOTFS_SIZE ?= "8192"

IMAGE_FSTYPES = "wic"
inherit core-image

# We get some parts from image-live that we need in order to boot from initrd
INITRD_IMAGE_LIVE ?= "core-image-tiny-initramfs"

python() {
    image_b = d.getVar('IMAGE_BASENAME')
    initrd_i = d.getVar('INITRD_IMAGE_LIVE')
    if image_b == initrd_i:
        bb.error('INITRD_IMAGE_LIVE %s cannot use the requested IMAGE_FSTYPE' % initrd_i)
        bb.fatal('Check IMAGE_FSTYPES and INITRAMFS_FSTYPES settings.')
    elif initrd_i:
        d.appendVarFlag('do_image', 'depends', ' %s:do_image_complete' % initrd_i)
}

WKS_FILE_intel-corei7-64 = "core-image-tiny.wks.in"
WKS_FILE_intel-core2-32 = "core-image-tiny.wks.in"
WKS_FILE_intel-quark = "mktinygalileodisk.wks"
