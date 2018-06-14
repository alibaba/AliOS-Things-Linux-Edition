SUMMARY = "Initramfs kernel boot"
DESCRIPTION = "This package includes the initramfs for the kernel boot. \
"
LICENSE = "MIT"

DEFAULT_PREFERENCE = "-1"

DEPENDS = "virtual/kernel"

PROVIDES = "virtual/kernel-initramfs"

ALLOW_EMPTY_${PN} = "1"

B = "${WORKDIR}/${BPN}-${PV}"

inherit linux-kernel-base kernel-arch

INITRAMFS_BASE_NAME = "${KERNEL_IMAGETYPE}-initramfs-${PV}-${PR}-${MACHINE}-${DATETIME}"
INITRAMFS_BASE_NAME[vardepsexclude] = "DATETIME"
INITRAMFS_EXT_NAME = "-${@oe.utils.read_file('${STAGING_KERNEL_BUILDDIR}/kernel-abiversion')}"

BUNDLE = "${@'1' if d.getVar('INITRAMFS_IMAGE', True) and \
                    d.getVar('INITRAMFS_IMAGE_BUNDLE', True) == '1' \
                 else '0'}"

python() {
    image = d.getVar('INITRAMFS_IMAGE', True)
    if image:
        d.appendVarFlag('do_install', 'depends', ' ${INITRAMFS_IMAGE}:do_rootfs')
}

do_unpack[depends] += "virtual/kernel:do_deploy"
do_populate_lic[depends] += "virtual/kernel:do_deploy"

do_install() {
    [ -z "${INITRAMFS_IMAGE}" ] && exit 0

    if [ "${BUNDLE}" = "0" ]; then
        for suffix in cpio.gz cpio.lzo cpio.lzma cpio.xz; do
            img="${DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE}-${MACHINE}.$suffix"

	    if [ -s "$img" ]; then
                install -d "${D}/boot"
                install -m 0644 "$img" \
                    "${D}/boot/${INITRAMFS_IMAGE}${INITRAMFS_EXT_NAME}.$suffix"
		break
            fi
	done
    else
	if [ -e "${DEPLOY_DIR_IMAGE}/${KERNEL_IMAGETYPE}-initramfs-${MACHINE}.bin" ]; then
	    install -d "${D}/boot"
            install -m 0644 "${DEPLOY_DIR_IMAGE}/${KERNEL_IMAGETYPE}-initramfs-${MACHINE}.bin" \
                "${D}/boot/${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}"
	fi
    fi
}

pkg_postinst_${PN}() {
    if [ "${BUNDLE}" = "1" ]; then
        update-alternatives --install "/boot/${KERNEL_IMAGETYPE}" \
            "${KERNEL_IMAGETYPE}" "/boot/${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}" \
            50101 || true
    fi
}

pkg_prerm_${PN}() {
    if [ "${BUNDLE}" = "1" ]; then
        update-alternatives --remove "${KERNEL_IMAGETYPE}" \
            "${KERNEL_IMAGETYPE}-initramfs${INITRAMFS_EXT_NAME}" || true
    fi
}

PACKAGE_ARCH = "${MACHINE_ARCH}"

FILES_${PN} = "/boot/*"
