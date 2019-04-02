DESCRIPTION = "AliOS Things Linux Edition Linkkit SDK"
SUMMARY = "AliOS Things Linux Edition Linkkit SDK 3.0.1"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=2a944942e1496af1886903d274dedb13"

SRC_URI = "file://linkkit-3.0.1.tar.gz;subdir=${S} \
              file://hal/ \
              file://config.alios.linux \
              file://make.settings \
              file://0001-report-linux-os-version-in-ClientID.patch \
              "

DEPENDS += "bc-native mbedtls"

python do_menuconfig() {
    oe_terminal("${SHELL} -c \"make menuconfig; printf 'press any key to continue...';read r;\"","linkkit configuration",d)
}

do_menuconfig[depends] += "ncurses-native:do_populate_sysroot"
do_menuconfig[nostamp] = "1"
do_menuconfig[dirs] = "${S}"
addtask menuconfig after do_configure

do_compile() {

    cp -rvf ${WORKDIR}/hal/alios ${S}/wrappers/os/
    cp -vf ${WORKDIR}/config.alios.linux ${S}/tools/board
    cp -vf ${WORKDIR}/make.settings ${S}/
    rm -rf ${S}/external_libs/mbedtls

    for cc in ${CC}
    do
        echo $cc | grep "\-gcc"
        if [ $? -eq 0 ]
        then
            sed -i "/CROSS_PREFIX =/d" ${S}/tools/board/config.alios.linux
            echo -n "CROSS_PREFIX = " >> ${S}/tools/board/config.alios.linux
            echo ${cc%gcc} >> ${S}/tools/board/config.alios.linux
            break
        fi
    done

    CFLAGS="${CC#* }" oe_runmake config DEFAULT_BLD="${S}/tools/board/config.alios.linux"
    CFLAGS="${CC#* }" oe_runmake DEFAULT_BLD="${S}/tools/board/config.alios.linux"
}

do_install_append() {
    install -d ${D}/${libdir}
    install -d ${D}/${includedir}

    install -m 0644 ${S}/output/release/lib/libiot_sdk.a ${D}/${libdir}/
    install -m 0644 ${S}/output/release/lib/libiot_hal.a ${D}/${libdir}/
    cp -f ${S}/output/release/include/*.h ${D}/${includedir}/
    cp -f ${S}/output/release/include/infra/*.h ${D}/${includedir}/
}

