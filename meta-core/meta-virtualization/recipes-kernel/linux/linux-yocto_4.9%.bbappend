FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto:"

# container support
SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', 'container', ' file://container.scc', '', d)}"

# KVM support
SRC_URI += " ${@bb.utils.contains('DISTRO_FEATURES', 'kvm', ' file://kvm-enable.scc', '', d)}"
#KERNEL_MODULE_AUTOLOAD += "${@bb.utils.contains('DISTRO_FEATURES', 'kvm', 'kvm', '', d)}"
#KERNEL_MODULE_AUTOLOAD += "${@bb.utils.contains('DISTRO_FEATURES', 'kvm', 'kvm-intel', '', d)}"
