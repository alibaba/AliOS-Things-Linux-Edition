# RPM_GPG_NAME and RPM_GPG_PASSPHRASE must be configured in your build
# environment. By default, the values for the sample keys are configured
# in meta-signing-key.

RPM_SIGN_FILES = "${@bb.utils.contains('DISTRO_FEATURES', 'ima', '1', '0', d)}"
# By default, the values below are applicable for the sample keys provided
# by meta-signing-key.
RPM_FSK_PATH ?= "${@uks_ima_keys_dir(d) + 'x509_ima.key'}"
RPM_FSK_PASSWORD ?= "password"

inherit sign_rpm user-key-store

python check_rpm_public_key () {
    gpg_path = d.getVar('GPG_PATH', True)
    gpg_bin = d.getVar('GPG_BIN', True) or \
              bb.utils.which(os.getenv('PATH'), 'gpg')
    gpg_keyid = d.getVar('RPM_GPG_NAME', True)

    # Check RPM_GPG_NAME and RPM_GPG_PASSPHRASE
    cmd = "%s --homedir %s --list-keys %s" % \
            (gpg_bin, gpg_path, gpg_keyid)
    status, output = oe.utils.getstatusoutput(cmd)
    if not status:
        return

    # Import RPM_GPG_NAME if not found
    gpg_key = uks_rpm_keys_dir(d) + 'RPM-GPG-PRIVKEY-' + gpg_keyid
    cmd = '%s --batch --homedir %s --passphrase %s --import %s' % \
            (gpg_bin, gpg_path, d.getVar('RPM_GPG_PASSPHRASE', True), gpg_key)
    status, output = oe.utils.getstatusoutput(cmd)
    if status:
        raise bb.build.FuncFailed('Failed to import gpg key (%s): %s' %
                                  (gpg_key, output))
}
check_rpm_public_key[lockfiles] = "${TMPDIR}/check_rpm_public_key.lock"
do_package_write_rpm[prefuncs] += "check_rpm_public_key"
check_rpm_public_key[prefuncs] += "check_deploy_keys"

python () {
    gpg_path = d.getVar('GPG_PATH', True)
    if not gpg_path:
        gpg_path = d.getVar('TMPDIR', True) + '/.gnupg'
        d.setVar('GPG_PATH', gpg_path)

    if not os.path.exists(gpg_path):
        status, output = oe.utils.getstatusoutput('mkdir -m 0700 -p %s' % gpg_path)
        if status:
            raise bb.build.FuncFailed('Failed to create gpg keying %s: %s' %
                                      (gpg_path, output))
}
