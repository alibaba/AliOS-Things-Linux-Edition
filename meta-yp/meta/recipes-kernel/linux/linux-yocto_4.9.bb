KBRANCH ?= "standard/base"

require recipes-kernel/linux/linux-yocto.inc

# board specific branches
KBRANCH_qemuarm  ?= "standard/arm-versatile-926ejs"
KBRANCH_qemuarm64 ?= "standard/qemuarm64"
KBRANCH_qemumips ?= "standard/mti-malta32"
KBRANCH_qemuppc  ?= "standard/qemuppc"
KBRANCH_qemux86  ?= "standard/base"
KBRANCH_qemux86-64 ?= "standard/base"
KBRANCH_qemumips64 ?= "standard/mti-malta64"

SRCREV_machine_qemuarm ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '432204c9d6baff2a41976ecc17bbc0170e86a6b2', '8caa35a74753d45178720933f03d8d5150a8ff17', d)}"
SRCREV_machine_qemuarm64 ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '99979fa22ff49400359558eb9e02bbf0e18a032f', '480ee599fb8df712c10dcf4b7aa6398b79f7d404', d)}"
SRCREV_machine_qemumips ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '13e618a8fded33a4339783a8a940acaf49084ab7', 'fc2a3b9f932779fdf053675a5a73e8f9917507a5', d)}"
SRCREV_machine_qemuppc ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '99979fa22ff49400359558eb9e02bbf0e18a032f', '480ee599fb8df712c10dcf4b7aa6398b79f7d404', d)}"
SRCREV_machine_qemux86 ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '99979fa22ff49400359558eb9e02bbf0e18a032f', '480ee599fb8df712c10dcf4b7aa6398b79f7d404', d)}"
SRCREV_machine_qemux86-64 ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '99979fa22ff49400359558eb9e02bbf0e18a032f', '480ee599fb8df712c10dcf4b7aa6398b79f7d404', d)}"
SRCREV_machine_qemumips64 ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '61867653815d2c359e739b5c93a7411d9acb1df9', 'aee63978005c04ea853099764acaa08130e65554', d)}"
SRCREV_machine ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '99979fa22ff49400359558eb9e02bbf0e18a032f', '480ee599fb8df712c10dcf4b7aa6398b79f7d404', d)}"
SRCREV_meta ?= "${@bb.utils.contains('LINUX_VERSION', '4.9.155', '9aed9998adc8848b33ade296422f7df7642bbc04', '6acae6f7200af17b3c2be5ecab2cffdc59a02b35', d)}"

SRC_URI = "git://git.yoctoproject.org/linux-yocto-4.9.git;name=machine;branch=${KBRANCH}; \
           git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-4.9;destsuffix=${KMETA}"

LINUX_VERSION ?= "4.9.155"

PV = "${LINUX_VERSION}+git${SRCPV}"

KMETA = "kernel-meta"
KCONF_BSP_AUDIT_LEVEL = "2"

KERNEL_DEVICETREE_qemuarm = "versatile-pb.dtb"

COMPATIBLE_MACHINE = "qemuarm|qemuarm64|qemux86|qemuppc|qemumips|qemumips64|qemux86-64"

# Functionality flags
KERNEL_EXTRA_FEATURES ?= "features/netfilter/netfilter.scc"
KERNEL_FEATURES_append = " ${KERNEL_EXTRA_FEATURES}"
KERNEL_FEATURES_append_qemuall=" cfg/virtio.scc"
KERNEL_FEATURES_append_qemux86=" cfg/sound.scc cfg/paravirt_kvm.scc"
KERNEL_FEATURES_append_qemux86-64=" cfg/sound.scc cfg/paravirt_kvm.scc"
KERNEL_FEATURES_append = " ${@bb.utils.contains("TUNE_FEATURES", "mx32", " cfg/x32.scc", "" ,d)}"
