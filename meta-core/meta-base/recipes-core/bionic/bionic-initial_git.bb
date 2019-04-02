DEPENDS = "virtual/${TARGET_PREFIX}gcc-initial"
PROVIDES = "virtual/${TARGET_PREFIX}libc-initial"

PACKAGES = ""
PACKAGES_DYNAMIC = ""

require bionic.inc

STAGINGCC = "gcc-cross-initial-${TARGET_ARCH}"
STAGINGCC_class-nativesdk = "gcc-crosssdk-initial-${SDK_SYS}"

PATH_prepend = "${STAGING_BINDIR_TOOLCHAIN}.${STAGINGCC}:"

do_compile () {
    :
}

do_install () {
    install -d ${D}${includedir}
    install -d ${D}${libdir}
    oe_runmake install-bionic-headers
}

inherit nopackages

deltask do_build