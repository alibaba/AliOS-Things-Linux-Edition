KBRANCH ?= "standard/tiny/common-pc"
LINUX_KERNEL_TYPE = "tiny"
KCONFIG_MODE = "--allnoconfig"

require recipes-kernel/linux/linux-yocto.inc

LINUX_VERSION ?= "4.9.155"

KMETA = "kernel-meta"
KCONF_BSP_AUDIT_LEVEL = "2"

SRCREV_machine ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '99979fa22ff49400359558eb9e02bbf0e18a032f', '480ee599fb8df712c10dcf4b7aa6398b79f7d404', d)}"
SRCREV_meta ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '9aed9998adc8848b33ade296422f7df7642bbc04', '6acae6f7200af17b3c2be5ecab2cffdc59a02b35', d)}"

PV = "${LINUX_VERSION}+git${SRCPV}"

SRC_URI = "git://git.yoctoproject.org/linux-yocto-4.9.git;branch=${KBRANCH};name=machine \
           git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-4.9;destsuffix=${KMETA}"

COMPATIBLE_MACHINE = "qemux86|qemux86-64"

# Functionality flags
KERNEL_FEATURES = ""

KERNEL_DEVICETREE_qemuarm = "versatile-pb.dtb"
