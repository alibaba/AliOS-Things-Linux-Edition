require efitools.inc

# The generated native binaries are used during native and target build
DEPENDS += "${BPN}-native gnu-efi openssl"

SRC_URI_append += "\
    file://LockDown-enable-the-enrollment-for-DBX.patch \
    file://LockDown-show-the-error-message-with-3-sec-timeout.patch \
    file://Makefile-do-not-build-signed-efi-image.patch \
    file://Build-DBX-by-default.patch \
    file://LockDown-disable-the-entrance-into-BIOS-setup-to-re-.patch \
    file://Fix-help2man-error.patch \
"

COMPATIBLE_HOST = '(i.86|x86_64).*-linux'

inherit user-key-store deploy

EXTRA_OEMAKE_append += "\
    INCDIR_PREFIX='${STAGING_DIR_TARGET}' \
    CRTPATH_PREFIX='${STAGING_DIR_TARGET}' \
    SIGN_EFI_SIG_LIST='${STAGING_BINDIR_NATIVE}/sign-efi-sig-list' \
    CERT_TO_EFI_SIG_LIST='${STAGING_BINDIR_NATIVE}/cert-to-efi-sig-list' \
    CERT_TO_EFI_HASH_LIST='${STAGING_BINDIR_NATIVE}/cert-to-efi-hash-list' \
    HASH_TO_EFI_SIG_LIST='${STAGING_BINDIR_NATIVE}/hash-to-efi-sig-list' \
    MYGUID='${UEFI_SIG_OWNER_GUID}' \
    HELP2MAN_PROG_PREFIX='${STAGING_BINDIR_NATIVE}' \
"

python do_prepare_signing_keys() {
    if d.expand('${UEFI_SB}') != '1':
        return

    # Prepare PK, KEK and DB for LockDown.efi.
    if uks_signing_model(d) in ('sample', 'user'):
        dir = uefi_sb_keys_dir(d)
    else:
        dir = d.expand('${SAMPLE_UEFI_SB_KEYS_DIR}/')

    import shutil

    for _ in ('PK', 'KEK', 'DB'):
        shutil.copyfile(dir + _ + '.crt', d.expand('${S}/') + _ + '.crt')
        shutil.copyfile(dir + _ + '.key', d.expand('${S}/') + _ + '.key')

    # Make sure LockDown.efi contains the DB and KEK from Microsoft.
    if "${@bb.utils.contains('DISTRO_FEATURES', 'msft', '1', '0', d)}" == '1':
        shutil.copyfile(d.expand('${MSFT_DB_CERT}'), d.expand('${S}/DB.crt'))
        shutil.copyfile(d.expand('${MSFT_KEK_CERT}'), d.expand('${S}/KEK.crt'))

    path = create_uefi_dbx(d)
    if path:
        with open(d.expand('${S}/DBX.crt'), 'w') as f:
            pass

        shutil.copyfile(path, d.expand('${S}/DBX.esl'))

        # Cheat the Makefile to avoid running this rule:
        # %.esl: %.crt cert-to-efi-sig-list
        #        $(CERT_TO_EFI_SIG_LIST) -g $(MYGUID) $< $@
        import time, os
        tm = time.strptime('2038-01-01 00:00:00', \
                           '%Y-%m-%d %H:%M:%S')
        time_stamp = time.mktime(tm)
        os.utime(d.expand('${S}/DBX.esl'), (time_stamp, time_stamp))
}
addtask prepare_signing_keys after do_configure before do_compile
do_prepare_signing_keys[prefuncs] += "check_deploy_keys"

do_install_append() {
    install -d ${D}${EFI_BOOT_PATH}
    install -m 0755 ${D}${datadir}/efitools/efi/LockDown.efi ${D}${EFI_BOOT_PATH}
}

do_deploy() {
    install -d ${DEPLOYDIR}

    install -m 0600 ${D}${EFI_BOOT_PATH}/LockDown.efi "${DEPLOYDIR}"
}
addtask deploy after do_install before do_build

RDEPENDS_${PN}_append += "\
    parted mtools coreutils util-linux openssl libcrypto \
"
