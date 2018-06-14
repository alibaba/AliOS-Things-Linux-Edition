DESCRIPTION = "A C++ library for interacting with JSON."
HOMEPAGE = "https://github.com/open-source-parsers/jsoncpp"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# V-1.6.0
SRC_URI = "file://cjson-1.6.0.tar.gz"

# Must change PR whenever the SRCREV is changed
PR = "r0"

S = "${WORKDIR}/cjson-1.6.0"

do_compile() {
    make all
}

do_install() {
    install -d ${D}${libdir}
    
    install -m 0755 ${S}/libcjson.so*       ${D}${libdir}
    install -m 0755 ${S}/libcjson_utils.so* ${D}${libdir}
}

FILES_${PN} += "${libdir}/*"

PACKAGES = "${PN} ${PN}-dbg ${PN}-staticdev ${PN}-dev"
