DESCRIPTION = "AliOS Things Linux Edition Linkkit SDK"
SUMMARY = "AliOS Things Linux Edition Linkkit SDK"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/iotx-sdk-c_clone/LICENSE;md5=f8a3df5abb62d05e864165fc8191e776"

SRC_URI = "file://linkkit-${PV}.tar.gz"

TARGET_CC_ARCH += "${LDFLAGS}"

do_install() {
    install -d ${D}/${bindir}
    install -d ${D}/${libdir}
    install -d ${D}/${includedir}

    # install example programs into the ${PN} package.
    install -m 0755 ${S}/iotx-sdk-c_clone/output/release/bin/* ${D}/${bindir}/

    install -m 0644 ${S}/iotx-sdk-c_clone/output/release/lib/* ${D}/${libdir}/
    cp -rf ${S}/iotx-sdk-c_clone/output/release/include/* ${D}/${includedir}/
    install -m 0644 ${S}/iotx-sdk-c_clone/src/infra/log/iotx_log.h ${D}/${includedir}
    install -m 0644 ${S}/iotx-sdk-c_clone/src/infra/log/iotx_log_config.h ${D}/${includedir}
    install -m 0644 ${S}/iotx-sdk-c_clone/src/services/linkkit/dm/iotx_dm.h ${D}/${includedir}
}

