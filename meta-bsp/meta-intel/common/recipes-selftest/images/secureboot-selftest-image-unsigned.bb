require recipes-core/images/core-image-minimal.bb

DEPENDS_remove = "grub-efi"

inherit uefi-comboapp

WKS_FILE = "generic-bootdisk.wks.in"

do_uefiapp_deploy_append() {
    for i in ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.boot*.efi; do
        target=`basename $i`
        target=`echo $target | sed -e 's/${IMAGE_LINK_NAME}.//'`

        cat > ${IMAGE_ROOTFS}/boot/startup.nsh << EOF
$target
reset
EOF
    break
    done
}
