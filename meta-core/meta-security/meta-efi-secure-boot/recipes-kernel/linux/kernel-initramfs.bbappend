inherit user-key-store deploy

# Always fetch the latest initramfs image
do_install[nostamp] = "1"

fakeroot python do_sign() {
    initramfs = None

    if d.getVar('BUNDLE', True) == '0':
        initramfs = d.expand('${D}/boot/${INITRAMFS_IMAGE}${INITRAMFS_EXT_NAME}.cpio.gz')
    else:
        initramfs = d.expand('${D}/boot/${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}')

    if initramfs == None or not os.path.exists(initramfs):
        return

    uks_sel_sign(initramfs, d)
}
addtask sign after do_install before do_deploy do_package
do_sign[prefuncs] += "check_deploy_keys"

do_deploy() {
    initramfs=""
    initramfs_dest=""

    if [ "${BUNDLE}" = "0" ]; then
        initramfs="${D}/boot/${INITRAMFS_IMAGE}${INITRAMFS_EXT_NAME}.cpio.gz"
        initramfs_dest="${DEPLOYDIR}/${INITRAMFS_IMAGE}-${MACHINE}.cpio.gz"
    else
        initramfs="${D}/boot/${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}"
        initramfs_dest="${DEPLOYDIR}/${KERNEL_IMAGETYPE}-initramfs-${MACHINE}.bin"
    fi

    if [ -f "$initramfs.p7b" ]; then
        install -d "${DEPLOYDIR}"

        install -m 0644 "$initramfs.p7b" "$initramfs_dest.p7b"
    fi
}
addtask deploy after do_install before do_build

pkg_postinst_${PN}_append() {
    if [ "${BUNDLE}" = "1" ] ; then
        update-alternatives --install "/boot/${KERNEL_IMAGETYPE}.p7b" \
            "${KERNEL_IMAGETYPE}.p7b" \
            "/boot/${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}.p7b" 50101
    fi

    true
}

pkg_prerm_${PN}_append() {
    if [ "${BUNDLE}" = "1" ] ; then
        update-alternatives --remove "${KERNEL_IMAGETYPE}.p7b" \
            "${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}.p7b"
    fi

    true
}
