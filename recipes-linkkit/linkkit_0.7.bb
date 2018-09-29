DESCRIPTION = "AliOS Things Linux Edition linkkitapp"
SUMMARY = "AliOS Things Linux Edition linkkitapp"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/iotx-sdk-c_clone/LICENSE;md5=f8a3df5abb62d05e864165fc8191e776"

SRC_URI = "file://linkkit-0.7.tar.gz \
           file://linkkit.init"

# only support sysvinit for now.
# inherit update-rc.d

# INITSCRIPT_NAME = "linkkit"
# INITSCRIPT_PARAMS = "defaults 87"

TARGET_CC_ARCH += "${LDFLAGS}"

sysroot_stage_all_append() {
    mkdir -p ${SYSROOT_DESTDIR}${includedir}
    sysroot_stage_dir ${S}/iotx-sdk-c_clone/output/release/include ${SYSROOT_DESTDIR}${includedir}
    mkdir -p ${SYSROOT_DESTDIR}${libdir}
    sysroot_stage_dir ${S}/iotx-sdk-c_clone/output/release/lib ${SYSROOT_DESTDIR}${libdir}
    cp ${S}/iotx-sdk-c_clone/src/infra/log/iotx_log.h ${SYSROOT_DESTDIR}${includedir}
    cp ${S}/iotx-sdk-c_clone/src/infra/log/iotx_log_config.h ${SYSROOT_DESTDIR}${includedir}
    cp ${S}/iotx-sdk-c_clone/src/services/linkkit/dm/iotx_dm.h ${SYSROOT_DESTDIR}${includedir}
}

do_install() {
    # create linkkit dir
    install -d ${D}/${bindir}/
    install -d ${D}/${INIT_D_DIR}/
    # install -m 0755 ${WORKDIR}/linkkit.init ${D}/${INIT_D_DIR}/linkkit
    install -m 0755 ${S}/iotx-sdk-c_clone/output/release/bin/linkkit-example-solo ${D}/${bindir}/linkkitapp
}
