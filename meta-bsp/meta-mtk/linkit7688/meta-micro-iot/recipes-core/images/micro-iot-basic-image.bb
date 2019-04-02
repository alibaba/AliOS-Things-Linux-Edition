require recipes-core/images/core-image-minimal.bb

DEPENDS = "linux-yocto-linkit7688 padjffs2-native"

IMAGE_FSTYPES = "squashfs-xz jffs2"
# IMAGE_FEATURES += "read-only-rootfs"

# EXTRA_IMAGECMD_jffs2_remove = "--pad"
EXTRA_IMAGECMD_jffs2 += "-x lzo -x zlib"

IMAGE_INSTALL += "mtd-utils iw"

do_prepare_sysupgrade() {
    cat ${DEPLOY_DIR_IMAGE}/uImage > ${DEPLOY_DIR_IMAGE}/sys
    padjffs2 ${DEPLOY_DIR_IMAGE}/sys 64
    dd if=${DEPLOY_DIR_IMAGE}/sys of=${DEPLOY_DIR_IMAGE}/_sys bs=1 count=$(expr $(stat -c %s ${DEPLOY_DIR_IMAGE}/sys) - 4 )
    cat ${DEPLOY_DIR_IMAGE}/${IMAGE_BASENAME}-${MACHINE}.jffs2 >> ${DEPLOY_DIR_IMAGE}/_sys
    padjffs2 ${DEPLOY_DIR_IMAGE}/_sys 64
    dd if=${DEPLOY_DIR_IMAGE}/_sys of=${DEPLOY_DIR_IMAGE}/__sys bs=1 count=$(expr $(stat -c %s ${DEPLOY_DIR_IMAGE}/_sys) - 4 )
    mv ${DEPLOY_DIR_IMAGE}/__sys ${DEPLOY_DIR_IMAGE}/sys
    rm ${DEPLOY_DIR_IMAGE}/_sys
}

addtask prepare_sysupgrade after image_complete
