FILESEXTRAPATHS_prepend_intel-x86-common := "${@ bb.utils.contains(\
    'IMAGE_FEATURES', 'secureboot', '${THISDIR}/files:', '', d)}"

do_install_prepend() {
    if [ "${@ bb.utils.contains('IMAGE_FEATURES', 'secureboot', 1, 0, d)}" \
        = "1" ]; then
        sed -i "/^rootfs_partuuid/c rootfs_partuuid=\"${DISK_SIGNATURE_UUID}\""\
            ${WORKDIR}/install-efi.sh
    fi
}