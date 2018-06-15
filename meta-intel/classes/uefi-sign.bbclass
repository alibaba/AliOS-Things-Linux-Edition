# By default, sign all .efi binaries in ${B} after compiling and before deploying
SIGNING_DIR ?= "${B}"
SIGNING_BINARIES ?= "*.efi"
SIGN_AFTER ?= "do_compile"
SIGN_BEFORE ?= "do_deploy"

python () {
    import os
    import hashlib

    # Ensure that if the signing key or cert change, we rerun the uefiapp process
    if bb.utils.contains('IMAGE_FEATURES', 'secureboot', True, False, d):
        for varname in ('SECURE_BOOT_SIGNING_CERT', 'SECURE_BOOT_SIGNING_KEY'):
            filename = d.getVar(varname)
            if filename is None:
                bb.fatal('%s is not set.' % varname)
            if not os.path.isfile(filename):
                bb.fatal('%s=%s is not a file.' % (varname, filename))
            with open(filename, 'rb') as f:
                data = f.read()
            hash = hashlib.sha256(data).hexdigest()
            d.setVar('%s_HASH' % varname, hash)

            # Must reparse and thus rehash on file changes.
            bb.parse.mark_dependency(d, filename)

        bb.build.addtask('uefi_sign', d.getVar('SIGN_BEFORE'), d.getVar('SIGN_AFTER'), d)

        # Original binary needs to be regenerated if the hash changes since we overwrite it
        # SIGN_AFTER isn't necessarily when it gets generated, but its our best guess
        d.appendVarFlag(d.getVar('SIGN_AFTER'), 'vardeps', 'SECURE_BOOT_SIGNING_CERT_HASH SECURE_BOOT_SIGNING_KEY_HASH')
}

do_uefi_sign() {
    if [ -f ${SECURE_BOOT_SIGNING_KEY} ] && [ -f ${SECURE_BOOT_SIGNING_CERT} ]; then
        for i in `find ${SIGNING_DIR}/ -name '${SIGNING_BINARIES}'`; do
            sbsign --key ${SECURE_BOOT_SIGNING_KEY} --cert ${SECURE_BOOT_SIGNING_CERT} $i
            sbverify --cert ${SECURE_BOOT_SIGNING_CERT} $i.signed
            mv $i.signed $i
        done
    fi
}

do_uefi_sign[depends] += "sbsigntool-native:do_populate_sysroot"

do_uefi_sign[vardeps] += "SECURE_BOOT_SIGNING_CERT_HASH \
                          SECURE_BOOT_SIGNING_KEY_HASH  \
                          SIGNING_BINARIES SIGNING_DIR  \
                          SIGN_BEFORE SIGN_AFTER        \
                         "
