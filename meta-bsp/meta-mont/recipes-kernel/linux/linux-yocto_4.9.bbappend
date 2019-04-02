FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}_${LINUX_VERSION}:"

PR := "${PR}.1"

# Confguration for mont-panther
SRC_URI += "file://defconfig \
            file://0001-panther-initial-patch.patch \
            ${@bb.utils.contains('LINUX_VERSION', '4.9.155', 'file://0002-Fix-Patche-conflicts.patch', '', d)} \
            file://tracepoints.cfg \
            file://modules.cfg \
            file://mont-panther.cfg \
            file://ltp.cfg \
           "
#KERNEL_IMAGETYPE = "uImage"
COMPATIBLE_MACHINE_mont-panther		= "mont-panther"

# We use kernel defconfigs where everything unmarked defaults to Y but 
# Yocto defaults to allnoconfig. This parameters makes them usable in yocto
KCONFIG_MODE = "--alldefconfig"
