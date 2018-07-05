DESCRIPTION = "Key store for key installation"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "\
    file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302 \
"

S = "${WORKDIR}"

inherit user-key-store

ALLOW_EMPTY_${PN} = "1"

KEY_DIR = "${sysconfdir}/keys"
# For RPM verification
RPM_KEY_DIR = "${sysconfdir}/pki/rpm-gpg"

# For ${PN}-system-trusted-privkey
SYSTEM_PRIV_KEY = "${KEY_DIR}/system_trusted_key.key"

# For ${PN}-extra-system-trusted-privkey
EXTRA_SYSTEM_PRIV_KEY = "${KEY_DIR}/extra_system_trusted_key.key"

# For ${PN}-modsign-privkey
MODSIGN_PRIV_KEY = "${KEY_DIR}/modsign_key.key"

# For ${PN}-ima-privkey
IMA_PRIV_KEY = "${KEY_DIR}/privkey_evm.crt"

# For ${PN}-system-trusted-cert
SYSTEM_CERT = "${KEY_DIR}/system_trusted_key.crt"

# For ${PN}-extra-system-trusted-cert
EXTRA_SYSTEM_CERT = "${KEY_DIR}/extra_system_trusted_key.crt"

# For ${PN}-modsign-cert
MODSIGN_CERT = "${KEY_DIR}/modsign_key.crt"

# For ${PN}-ima-cert
IMA_CERT = "${KEY_DIR}/x509_evm.der"

python () {
    if not (uks_signing_model(d) in "sample", "user"):
        return

    pn = d.getVar('PN', True) + '-system-trusted-privkey'
    d.setVar('PACKAGES_prepend', pn + ' ')
    d.setVar('FILES_' + pn, d.getVar('SYSTEM_PRIV_KEY', True))
    d.setVar('CONFFILES_' + pn, d.getVar('SYSTEM_PRIV_KEY', True))

    pn = d.getVar('PN', True) + '-extra-system-trusted-privkey'
    d.setVar('PACKAGES_prepend', pn + ' ')
    d.setVar('FILES_' + pn, d.getVar('EXTRA_SYSTEM_PRIV_KEY', True))
    d.setVar('CONFFILES_' + pn, d.getVar('EXTRA_SYSTEM_PRIV_KEY', True))

    pn = d.getVar('PN', True) + '-modsign-privkey'
    d.setVar('PACKAGES_prepend', pn + ' ')
    d.setVar('FILES_' + pn, d.getVar('MODSIGN_PRIV_KEY', True))
    d.setVar('CONFFILES_' + pn, d.getVar('MODSIGN_PRIV_KEY', True))

    pn = d.getVar('PN', True) + 'ima-privkey'
    d.setVar('PACKAGES_prepend', pn + ' ')
    d.setVar('FILES_' + pn, d.getVar('IMA_PRIV_KEY', True))
    d.setVar('CONFFILES_' + pn, d.getVar('IMA_PRIV_KEY', True))

    pn = d.getVar('PN', True) + '-rpm-pubkey'
    d.setVar('PACKAGES_prepend', pn + ' ')
    d.setVar('FILES_' + pn, d.getVar('RPM_KEY_DIR', True) + '/RPM-GPG-KEY-' + d.getVar('RPM_GPG_NAME', True))
    d.setVar('CONFFILES_' + pn, d.getVar('RPM_KEY_DIR', True) + '/RPM-GPG-KEY-' + d.getVar('RPM_GPG_NAME', True))
    d.appendVar('RDEPENDS_' + pn, ' rpm')
}

do_install() {
    install -d "${D}${RPM_KEY_DIR}"

    for f in `ls ${WORKDIR}/RPM-GPG-KEY-* 2>/dev/null`; do
        [ ! -f "$f" ] && continue

        install -m 0644 "$f" "${D}${RPM_KEY_DIR}"
    done

    key_dir="${@uks_rpm_keys_dir(d)}"
    if [ -n "$key_dir" ]; then
        for f in `ls $key_dir/RPM-GPG-KEY-* 2>/dev/null`; do
            [ ! -s "$f" ] && continue

            install -m 0644 "$f" "${D}${RPM_KEY_DIR}"
        done
    fi

    install -d "${D}${KEY_DIR}"

    key_dir="${@uks_system_trusted_keys_dir(d)}"
    install -m 0644 "$key_dir/system_trusted_key.crt" "${D}${SYSTEM_CERT}"

    if [ "${@uks_signing_model(d)}" = "sample" -o "${@uks_signing_model(d)}" = "user" ]; then
        install -m 0400 "$key_dir/system_trusted_key.key" "${D}${SYSTEM_PRIV_KEY}"
    fi

    key_dir="${@uks_extra_system_trusted_keys_dir(d)}"
    install -m 0644 "$key_dir/extra_system_trusted_key.crt" \
        "${D}${EXTRA_SYSTEM_CERT}"

    if [ "${@uks_signing_model(d)}" = "sample" -o "${@uks_signing_model(d)}" = "user" ]; then
        install -m 0400 "$key_dir/extra_system_trusted_key.key" \
            "${D}${EXTRA_SYSTEM_PRIV_KEY}"
    fi

    key_dir="${@uks_modsign_keys_dir(d)}"
    install -m 0644 "$key_dir/modsign_key.crt" \
        "${D}${MODSIGN_CERT}"

    if [ "${@uks_signing_model(d)}" = "sample" -o "${@uks_signing_model(d)}" = "user" ]; then
        install -m 0400 "$key_dir/modsign_key.key" \
            "${D}${MODSIGN_PRIV_KEY}"
    fi

    key_dir="${@uks_ima_keys_dir(d)}"
    install -m 0644 "$key_dir/x509_ima.der" "${D}${IMA_CERT}"

    if [ "${@uks_signing_model(d)}" = "sample" -o "${@uks_signing_model(d)}" = "user" ]; then
        install -m 0400 "$key_dir/x509_ima.key" "${D}${IMA_PRIV_KEY}"
    fi
}

do_install[prefuncs] += "check_deploy_keys"

SYSROOT_PREPROCESS_FUNCS += "key_store_sysroot_preprocess"

key_store_sysroot_preprocess() {
    sysroot_stage_dir "${D}${sysconfdir}" "${SYSROOT_DESTDIR}${sysconfdir}"
}

pkg_postinst_${PN}-rpm-pubkey() {
    if [ -z "$D" ]; then
        keydir="${RPM_KEY_DIR}"

        [ ! -d "$keydir" ] && mkdir -p "$keydir"

        # XXX: only import the new key
        for keyfile in `ls $keydir/RPM-GPG-KEY-*`; do
            [ -s "$keyfile" ] || continue

            rpm --import "$keyfile" || {
                echo "Unable to import the public key $keyfile"
                exit 1
            }
        done
    fi
}

PACKAGES = "\
    ${PN}-system-trusted-cert \
    ${PN}-extra-system-trusted-cert \
    ${PN}-modsign-cert \
    ${PN}-ima-cert \
"

# Note any private key is not available if user key signing model used.
PACKAGES_DYNAMIC = "\
    ${PN}-system-trusted-privkey \
    ${PN}-extra-system-trusted-privkey \
    ${PN}-modsign-privkey \
    ${PN}-ima-privkey \
    ${PN}-rpm-pubkey \
"

FILES_${PN}-system-trusted-cert = "${SYSTEM_CERT}"
CONFFILES_${PN}-system-trusted-cert = "${SYSTEM_CERT}"

FILES_${PN}-extra-system-trusted-cert = "${EXTRA_SYSTEM_CERT}"
CONFFILES_${PN}-extra-system-trusted-cert = "${EXTRA_SYSTEM_CERT}"

FILES_${PN}-modsign-cert = "${MODSIGN_CERT}"
CONFFILES_${PN}-modsign-cert = "${MODSIGN_CERT}"

FILES_${PN}-ima-cert = "${IMA_CERT}"
CONFFILES_${PN}-ima-cert = "${IMA_CERT}"
