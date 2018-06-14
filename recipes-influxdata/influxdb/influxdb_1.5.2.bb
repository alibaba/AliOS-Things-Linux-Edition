DESCRIPTION = "InfluxDB is an open source time series database with no external dependencies. It's useful for recording metrics, events, and performing analytics."

SECTION = "examples"
HOMEPAGE = "https://www.influxdata.com"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI = "file://${PN}-${PV}.tar.bz2"
SRC_URI += "file://${PN}-vendor-${PV}.tar.bz2"
SRC_URI += "file://0001-skip-dependency-check-routine.patch"

GO_IMPORT = "github.com/influxdata/influxdb"

inherit go

RDEPENDS_${PN} += "go-runtime"
DEPENDS += "patchelf-native"

do_configure[noexec] = "1"
do_compile_ptest_base[noexec] = "1"

do_compile() {
    cd ${S}
    ./build.py --verbose --outdir ${WORKDIR}/bin
}

do_install() {
    cp -rf ${THISDIR}/files/etc ${D}
    cp -rf ${THISDIR}/files/usr ${D}
    cp -rf ${THISDIR}/files/var ${D}
    cp -rf ${WORKDIR}/bin ${D}/usr/

    cd ${D}/usr/bin
    for iter in *; do
        patchelf --set-interpreter `linuxloader_glibc` ${iter}
    done
}

do_prepare_source() {
    cd ${GOPATH}/src
    cp -rf ${S} ${GOPATH}/src/github.com/influxdata/influxdb
}
addtask do_prepare_source after do_unpack before do_patch

FILES_${PN} += "/*"
