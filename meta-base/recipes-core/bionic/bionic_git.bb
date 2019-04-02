require bionic.inc

PROVIDES = "virtual/libc virtual/${TARGET_PREFIX}libc-for-gcc"

DEPENDS = "virtual/${TARGET_PREFIX}binutils \
           virtual/${TARGET_PREFIX}gcc-initial \
           libgcc-initial \
           linux-libc-headers \
           virtual/${TARGET_PREFIX}libc-initial \
          "

STAGINGCC = "gcc-cross-initial-${TARGET_ARCH}"
STAGINGCC_class-nativesdk = "gcc-crosssdk-initial-${SDK_SYS}"

PATH_prepend = "${STAGING_BINDIR_TOOLCHAIN}.${STAGINGCC}:"

do_compile () {
    oe_runmake libs
}

do_install () {
    install -d ${D}${libdir}
    install -d ${D}${includedir}
    oe_runmake install-bionic-headers
    oe_runmake install-libs
    for l in crypt pthread resolv rt util xnet
    do
        ln -srf ${D}${libdir}/libc.so ${D}${libdir}/lib$l.so
    done
}

do_stash_locale() {
    :
}

do_siteconfig () {
    :
}

FILES_${PN} += "${libdir}/lib*"
RPROVIDES_${PN}-dev += "libc-dev virtual-libc-dev"