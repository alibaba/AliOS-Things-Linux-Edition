FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto:"

IMA_ENABLED = "${@bb.utils.contains('DISTRO_FEATURES', 'ima', '1', '0', d)}"
MODSIGN_ENABLED = "${@bb.utils.contains('DISTRO_FEATURES', 'modsign', '1', '0', d)}"

DEPENDS += "${@'key-store openssl-native' \
               if d.getVar('IMA_ENABLED', True) == '1' or \
                  d.getVar('MODSIGN_ENABLED', True) == '1' \
               else ''}"

SRC_URI += "\
    ${@'file://ima.scc file://ima.cfg' \
       if d.getVar('IMA_ENABLED', True) == '1' else ''} \
    ${@'file://modsign.scc file://modsign.cfg' \
       if d.getVar('MODSIGN_ENABLED', True) == '1' else ''} \
"

INHIBIT_PACKAGE_STRIP = "${@'1' if d.getVar('MODSIGN_ENABLED', True) == '1' else '0'}"

do_configure_prepend() {
    sys_cert="${STAGING_DIR_TARGET}${sysconfdir}/keys/system_trusted_key.crt"
    modsign_key="${STAGING_DIR_TARGET}${sysconfdir}/keys/modsign_key.key"
    modsign_cert="${STAGING_DIR_TARGET}${sysconfdir}/keys/modsign_key.crt"

    if [ -f "$sys_cert" ]; then
        install -m 0644 "$sys_cert" "${B}"
    fi

    if [ -f "$modsign_key" -a -f "$modsign_cert" ]; then
        cat "$modsign_key" "$modsign_cert" \
            > "${B}/modsign_key.pem"
    else
        true
    fi
}
