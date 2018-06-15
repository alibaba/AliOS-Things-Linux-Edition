# rmc-boot bbclass
# Deploy central RMC database file to ESP

IMAGE_INSTALL_append = " rmc"
RMC_BOOTLOADER ?= "systemd-boot"

inherit ${RMC_BOOTLOADER}

do_bootimg[depends] += "${MLPREFIX}rmc-db:do_deploy"

efi_populate_append() {
	if [ -f ${DEPLOY_DIR_IMAGE}/rmc.db ]; then
		install -m 0400 ${DEPLOY_DIR_IMAGE}/rmc.db ${DEST}/rmc.db
	else
		rm -f ${DEST}/rmc.db
	fi
}
