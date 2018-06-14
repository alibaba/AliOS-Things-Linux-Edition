FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto:"

SRC_URI += "\
        ${@bb.utils.contains('DISTRO_FEATURES', 'apparmor', ' file://apparmor.cfg', '', d)} \
"
