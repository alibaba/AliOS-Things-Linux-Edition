addtask sd_installer

do_sd_installer() {
	local install_dir="${DEPLOY_DIR_IMAGE}/${IMAGE_BASENAME}--sd_installer"

	rm -fr ${install_dir}
	mkdir -p ${install_dir}/image/boot
	
	# sd installer files
	cp -arfL ${TOPDIR}/../meta-bsp/meta-via/tools/sd_installer/* ${install_dir}/

	# rootfs
	cp -arfL ${IMGDEPLOYDIR}/${IMAGE_LINK_NAME}.tar.gz ${install_dir}/image/rootfs.tgz

    # u-boot
	cp -arfL ${DEPLOY_DIR_IMAGE}/u-boot.imx ${install_dir}/image/u-boot.bin

	# kernel, dtb.
	cp -arfL ${DEPLOY_DIR_IMAGE}/zImage ${install_dir}/image/boot/
	cp -arfL ${DEPLOY_DIR_IMAGE}/imx6*dtb ${install_dir}/image/boot/
}

do_image_complete[postfuncs] += "do_sd_installer"
