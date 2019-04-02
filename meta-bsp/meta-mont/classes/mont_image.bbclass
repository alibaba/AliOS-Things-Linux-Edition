inherit image_types
#inherit kernel-artifact-names

IMAGE_BOOTLOADER = "boot-mont-panther-1.0-r0.img"
CI_OFFSET = "0xA0000"
IMG_RFS_ALIGN = "131072"


do_image_mont_image[recrdeps] = "do_build"

do_image_mont_image[depends] += " \
    mtd-utils-native:do_populate_sysroot \
    squashfs-tools-native:do_populate_sysroot \
	virtual/kernel:do_deploy \
	mont-boot:do_deploy \
    "

IMAGE_CMD_mont_image () {
	#jffs2------------------------------------------------------------------------
	#rootfs
	mkfs.jffs2 --root=${IMAGE_ROOTFS} --faketime --output=${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.jffs2_temp
	#kernel and rootfs
	KNL_CNT=`wc -c < ${DEPLOY_DIR_IMAGE}/uImage`
	KNL_CNT=`expr \( ${KNL_CNT} + ${IMG_RFS_ALIGN} - 1 \) / ${IMG_RFS_ALIGN}`
	dd if=${DEPLOY_DIR_IMAGE}/uImage of=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.jffs2.img
	dd if=${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.jffs2_temp of=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.jffs2.img bs=${IMG_RFS_ALIGN} seek=${KNL_CNT}
	rm -f ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.jffs2_temp
	#boot kernel and rootfs
	dd if=${DEPLOY_DIR_IMAGE}/${IMAGE_BOOTLOADER} of=${IMGDEPLOYDIR}/${IMAGE_NAME}.all.jffs2.img
	dd if=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.jffs2.img of=${IMGDEPLOYDIR}/${IMAGE_NAME}.all.jffs2.img bs=${IMG_RFS_ALIGN} seek=5

	#ubifs nor--------------------------------------------------------------------
	#rootfs
	echo \[ubifs\] > ${IMGDEPLOYDIR}/ubinize.nor.cfg
	echo mode=ubi >> ${IMGDEPLOYDIR}/ubinize.nor.cfg
	echo image=${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nor.ubifs_temp >> ${IMGDEPLOYDIR}/ubinize.nor.cfg
	echo vol_id=0 >> ${IMGDEPLOYDIR}/ubinize.nor.cfg
    echo vol_type=dynamic >> ${IMGDEPLOYDIR}/ubinize.nor.cfg
    echo vol_name=rootfs >> ${IMGDEPLOYDIR}/ubinize.nor.cfg
	echo vol_flags=autoresize >> ${IMGDEPLOYDIR}/ubinize.nor.cfg
	mkfs.ubifs -v -F -m 256 -e 65024 -x zlib -U -c 1024 -o ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nor.ubifs_temp -d ${IMAGE_ROOTFS}
	ubinize -o ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nor.ubi_temp -v -m 256 -p 64KiB -s 256 -O 256 ${IMGDEPLOYDIR}/ubinize.nor.cfg
	rm -f ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nor.ubifs_temp
    #kernel and rootfs
    KNL_CNT=`wc -c < ${DEPLOY_DIR_IMAGE}/uImage`
    KNL_CNT=`expr \( ${KNL_CNT} + ${IMG_RFS_ALIGN} - 1 \) / ${IMG_RFS_ALIGN}`
    dd if=${DEPLOY_DIR_IMAGE}/uImage of=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.nor.ubi.img
    dd if=${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nor.ubi_temp of=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.nor.ubi.img bs=${IMG_RFS_ALIGN} seek=${KNL_CNT}
    rm -f ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nor.ubi_temp
	#boot kernel and rootfs
	dd if=${DEPLOY_DIR_IMAGE}/${IMAGE_BOOTLOADER} of=${IMGDEPLOYDIR}/${IMAGE_NAME}.all.nor.ubi.img
	dd if=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.nor.ubi.img of=${IMGDEPLOYDIR}/${IMAGE_NAME}.all.nor.ubi.img bs=${IMG_RFS_ALIGN} seek=5

	#ubifs nand-------------------------------------------------------------------
	#rootfs
    echo \[ubifs\] > ${IMGDEPLOYDIR}/ubinize.nand.cfg
    echo mode=ubi >> ${IMGDEPLOYDIR}/ubinize.nand.cfg
    echo image=${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nand.ubifs_temp >> ${IMGDEPLOYDIR}/ubinize.nand.cfg
    echo vol_id=0 >> ${IMGDEPLOYDIR}/ubinize.nand.cfg
    echo vol_type=dynamic >> ${IMGDEPLOYDIR}/ubinize.nand.cfg
    echo vol_name=rootfs >> ${IMGDEPLOYDIR}/ubinize.nand.cfg
    echo vol_flags=autoresize >> ${IMGDEPLOYDIR}/ubinize.nand.cfg
	mkfs.ubifs -v -F -m 2048 -e 126976 -x zlib -U -c 512 -o ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nand.ubifs_temp -d ${IMAGE_ROOTFS}
	ubinize -o ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nand.ubi_temp -v -m 2048 -p 128KiB -s 2048 -O 2048 ${IMGDEPLOYDIR}/ubinize.nand.cfg
	rm -f ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nand.ubifs_temp
    #kernel and rootfs
    KNL_CNT=`wc -c < ${DEPLOY_DIR_IMAGE}/uImage`
    KNL_CNT=`expr \( ${KNL_CNT} + ${IMG_RFS_ALIGN} - 1 \) / ${IMG_RFS_ALIGN}`
    dd if=${DEPLOY_DIR_IMAGE}/uImage of=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.nand.ubi.img
    dd if=${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nand.ubi_temp of=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.nand.ubi.img bs=${IMG_RFS_ALIGN} seek=${KNL_CNT}
    rm -f ${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.nand.ubi_temp
	#boot kernel and rootfs
	dd if=${DEPLOY_DIR_IMAGE}/${IMAGE_BOOTLOADER} of=${IMGDEPLOYDIR}/${IMAGE_NAME}.all.nand.ubi.img
	dd if=${IMGDEPLOYDIR}/${IMAGE_NAME}.KNL_RFS.nand.ubi.img of=${IMGDEPLOYDIR}/${IMAGE_NAME}.all.nand.ubi.img bs=${IMG_RFS_ALIGN} seek=5

}
