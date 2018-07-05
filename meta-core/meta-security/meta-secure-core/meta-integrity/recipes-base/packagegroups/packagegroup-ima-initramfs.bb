DESCRIPTION = "Linux Integrity Measurement Architecture (IMA) subsystem for initramfs"

include packagegroup-ima.inc

RDEPENDS_${PN} += "\
    initrdscripts-ima \
"
