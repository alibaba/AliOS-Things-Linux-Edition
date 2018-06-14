DESCRIPTION = "A C++ library for interacting with JSON."
HOMEPAGE = "https://github.com/open-source-parsers/jsoncpp"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# V-1.8.4
SRCREV = "ddabf50f72cf369bf652a95c4d9fe31a1865a781"
SRC_URI = "git://github.com/open-source-parsers/jsoncpp.git"

# Must change PR whenever the SRCREV is changed
PR = "r3"

S = "${WORKDIR}/git"

LDFLAGS += "-fPIC -shared"

do_compile() {
    /usr/bin/python ${S}/amalgamate.py
    
    cd ${S}/dist
    ${CXX} jsoncpp.cpp -I. -o libjsoncpp.so ${LDFLAGS} 
}

do_install() {
    install -d ${D}${includedir}
    install -d ${D}${includedir}/json
    install -d ${D}${libdir}
    
    install -m 0755 ${S}/dist/libjsoncpp.so ${D}${libdir}
    
    for f in ${S}/dist/json/*.h; do
        install -m 0644 ${f} ${D}${includedir}/json
    done
}

FILES_${PN} += "${libdir}/* ${includedir}/json/*"

PACKAGES = "${PN} ${PN}-dbg ${PN}-staticdev ${PN}-dev"
