# Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require conf/machine/include/soc-family.inc

MACHINE_EXTRA_RRECOMMENDS_append = " kernel-modules linux-firmware"

SERIAL_CONSOLES ?= "115200;ttyFIQ0"
KERNEL_IMAGETYPE ?= "Image"
KBUILD_DEFCONFIG ?= "rockchip_linux_defconfig"

PREFERRED_PROVIDER_virtual/bootloader ?= "u-boot-rockchip"
PREFERRED_PROVIDER_u-boot ?= "u-boot-rockchip"

IMAGE_FSTYPES += "ext4"
IMAGE_FSTYPES_remove = "iso live"

RK_MINILOADER_INI ?= "${@d.getVar('SOC_FAMILY').upper() + 'MINIALL.ini'}"

IMAGE_FSTYPES += "wic"
WKS_FILE ?= "${@bb.utils.contains('DISTRO_FEATURES', 'uota', 'mender-gptdisk.wks', 'generic-gptdisk.wks', d)}"
do_image_wic[depends] += " \
        virtual/kernel:do_deploy \
        virtual/bootloader:do_deploy \
        rk-binary-loader:do_deploy \
"
export RK_ROOTDEV_UUID ?= "614e0000-0000-4b53-8000-1d28000054a9"

POST_ROOTFS_SCRIPT ?= "${RK_OVERLAY_DIR}/${MACHINE}/post_rootfs.sh"

ROOTFS_POSTPROCESS_COMMAND += "do_post_rootfs;"
do_post_rootfs() {
	if [ ! -f "${POST_ROOTFS_SCRIPT}" ];then
		echo "${POST_ROOTFS_SCRIPT} not found."
		return
	fi

	echo "Running ${POST_ROOTFS_SCRIPT}..."

	cd ${POST_ROOTFS_SCRIPT}/../
	./post_rootfs.sh ${IMAGE_ROOTFS}
}

# Pack data.tar & data/ to data.ext4
ROOTFS_POSTPROCESS_COMMAND += "${@bb.utils.contains('DISTRO_FEATURES', 'uota', 'gen_data;', '', d)}"
gen_data() {
	cd "${DEPLOY_DIR_IMAGE}"
	mkdir -p data

	[ -f data.tar ] && tar xf data.tar

	dd if=/dev/null of=data.ext4 bs=${RK_DATA_SIZE}K count=0 seek=1
	mke2fs -F data.ext4 -d data -L data
}

IMAGE_POSTPROCESS_COMMAND += "gen_rkparameter;"
gen_rkparameter() {
	IMAGE="${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.wic"
	if [ ! -f "${IMAGE}" ];then
		echo "${IMAGE} not found."
		return
	fi

	OUT="${IMGDEPLOYDIR}/parameter"

	echo "Generating ${OUT}..."

	echo "# IMAGE_NAME: ${IMAGE_NAME}" > ${OUT}
	echo "FIRMWARE_VER: 1.0" >> ${OUT}
	echo "TYPE: GPT" >> ${OUT}
	echo -n "CMDLINE: mtdparts=rk29xxnand:" >> ${OUT}
	sgdisk -p ${IMAGE} |grep -E "^ +[0-9]" |while read line;do
		NAME=$(echo ${line}|cut -f 7 -d ' ')
		START=$(echo ${line}|cut -f 2 -d ' ')
		END=$(echo ${line}|cut -f 3 -d ' ')
		SIZE=$(expr ${END} - ${START} + 1)
		printf "0x%08x@0x%08x(%s)," ${SIZE} ${START} ${NAME} >> ${OUT}
	done
	echo "\nuuid: rootfs=${RK_ROOTDEV_UUID}" >> ${OUT}
}

IMAGE_POSTPROCESS_COMMAND += "gen_rkupdateimg;"
gen_rkupdateimg() {
	IMAGE="${IMGDEPLOYDIR}/${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.wic"
	if [ ! -f "${IMAGE}" ];then
		echo "${IMAGE} not found."
		return
	fi

	cd ${IMGDEPLOYDIR}

	RK_IMAGES="loader.bin uboot.env u-boot-rk.img trust.img kernel.img \
			resource.img data.ext4"

	# Create temporary symlinks, because the tool would crash with abs pathes
	for img in ${RK_IMAGES};do
		f=${DEPLOY_DIR_IMAGE}/${img}
		[ -f ${f} ] && ln -sf ${f} .
	done

	ln -sf ${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.ext4 rootfs.img

	OUT="${IMGDEPLOYDIR}/package-file"

	echo "Generating ${OUT}..."

	echo "# IMAGE_NAME: ${IMAGE_NAME}" > ${OUT}
	echo "package-file package-file" >> ${OUT}
	echo "bootloader loader.bin" >> ${OUT}
	echo "parameter parameter" >> ${OUT}
	sgdisk -p ${IMAGE} |grep -E "^ +[0-9]" |while read line;do
		NAME=$(echo ${line}|cut -f 7 -d ' ')

		echo -n "${NAME} " >> ${OUT}
		case "${NAME}" in
			uboot-env)
				echo "uboot.env" >> ${OUT}
				;;
			uboot)
				echo "u-boot-rk.img" >> ${OUT}
				;;
			trust|kernel|resource)
				echo "${NAME}.img" >> ${OUT}
				;;
			root*)
				echo "rootfs.img" >> ${OUT}
				;;
			data)
				echo "data.ext4" >> ${OUT}
				;;
		esac
	done

	afptool -pack ./ update.raw.img
	rkImageMaker -${SOC_FAMILY} loader.bin update.raw.img update.img -os_type:androidos

	rm -rf ${RK_IMAGES} update.raw.img
}

# Fixup rootfs size to 240M
RK_ROOTFS_SIZE ?= "245760"
IMAGE_ROOTFS_ALIGNMENT = "${RK_ROOTFS_SIZE}"
IMAGE_ROOTFS_MAXSIZE = "${RK_ROOTFS_SIZE}"
IMAGE_ROOTFS_SIZE = "8192"
IMAGE_OVERHEAD_FACTOR = "1"
