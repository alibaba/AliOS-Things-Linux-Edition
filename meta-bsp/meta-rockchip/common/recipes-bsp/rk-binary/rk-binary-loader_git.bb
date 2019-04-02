# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Rockchip binary loader"

LICENSE = "LICENSE.rockchip"
LIC_FILES_CHKSUM = "file://${RK_BINARY_LICENSE};md5=5fd70190c5ed39734baceada8ecced26"

DEPENDS = "u-boot-mkimage-native rk-binary-native"

SRC_URI = "git://github.com/rockchip-linux/rkbin.git"
SRCREV = "3da9d346375ff4a4bcb63ca635c81c78634924ab"
S = "${WORKDIR}/git"

# Check needed variables
python () {
    if not d.getVar('RK_MINILOADER_INI'):
        raise bb.parse.SkipPackage('RK_MINILOADER_INI is not specified!')
    if not d.getVar('RK_TRUST_INI'):
        raise bb.parse.SkipPackage('RK_TRUST_INI is not specified!')
    if d.getVar('RK_TRUST_INI').endswith('TOS.ini') and not d.getVar('RK_TEE_ADDR'):
        raise bb.parse.SkipPackage('RK_TEE_ADDR is not specified!')
}

inherit deploy

RK_IDBLOCK_IMG = "idblock.img"
RK_LOADER_BIN = "loader.bin"
RK_TRUST_IMG = "trust.img"

do_compile() {
        FLASH_DATA=`grep "FlashData=" "RKBOOT/${RK_MINILOADER_INI}" | cut -d'=' -f2`
        FLASH_BOOT=`grep "FlashBoot=" "RKBOOT/${RK_MINILOADER_INI}" | cut -d'=' -f2`

        bbnote "${PN}: Generating ${RK_IDBLOCK_IMG} from ${FLASH_DATA} and ${FLASH_BOOT} for ${SOC_FAMILY}"
        mkimage -n "${SOC_FAMILY}" -T rksd -d "${FLASH_DATA}" "${RK_IDBLOCK_IMG}"
        cat "${FLASH_BOOT}" >> "${RK_IDBLOCK_IMG}"

        bbnote "${PN}: Generating ${RK_LOADER_BIN} from ${RK_MINILOADER_INI}"
	boot_merger --replace tools/rk_tools/ ./ "RKBOOT/${RK_MINILOADER_INI}"
	ln -sf "${SOC_FAMILY}"_loader*.bin "${RK_LOADER_BIN}"

        bbnote "${PN}: Generating ${RK_TRUST_IMG} from ${RK_TRUST_INI}"
        if echo ${RK_TRUST_INI} | grep "TOS.ini"; then
                TOS=`grep "TOS=" "${RK_TRUST_INI}" | cut -d'=' -f2 | sed "s#tools/rk_tools/#./#"`
                TOSTA=`grep "TOSTA=" "${RK_TRUST_INI}" | cut -d'=' -f2 | sed "s#tools/rk_tools/#./#"`
                if [ -n "${TOS}" ]; then
                        loaderimage --pack --trustos "${TOS}" "${RK_TRUST_IMG}" "${RK_TEE_ADDR}" --size 1024 1
                else
                        loaderimage --pack --trustos "${TOSTA}" "${RK_TRUST_IMG}" "${RK_TEE_ADDR}" --size 1024 1
                fi
        else
                trust_merger --replace tools/rk_tools/ ./ "RKTRUST/${RK_TRUST_INI}" --size 1024 1
        fi
}

do_deploy () {
        install -d "${DEPLOYDIR}"
        for binary in "${RK_IDBLOCK_IMG}" "${RK_LOADER_BIN}" "${RK_TRUST_IMG}";do
                install "${binary}" "${DEPLOYDIR}/${binary}-${SRCREV}"
                ln -sf "${binary}-${SRCREV}" "${DEPLOYDIR}/${binary}"
        done
}
addtask deploy before do_build after do_compile
