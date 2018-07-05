FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto:"

SRC_URI += "\
    ${@bb.utils.contains('DISTRO_FEATURES', 'tpm', \
                         'file://tpm.scc file://tpm.cfg', '', d)} \
"
