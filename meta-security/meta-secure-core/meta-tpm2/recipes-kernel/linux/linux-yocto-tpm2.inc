FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto:"

SRC_URI += "\
    ${@bb.utils.contains('DISTRO_FEATURES', 'tpm2', \
                         'file://tpm2.scc file://tpm2.cfg', '', d)} \
"
