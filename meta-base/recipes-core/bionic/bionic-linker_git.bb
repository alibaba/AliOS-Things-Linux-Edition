require bionic.inc

RPROVIDES_${PN} = " rtld(GNU_HASH)"
DEPENDS = "gcc-runtime \
           virtual/libc \
           virtual/${TARGET_PREFIX}gcc-initial \
           libgcc virtual/${TARGET_PREFIX}binutils \
          "

PATH_prepend = "${STAGING_BINDIR_TOOLCHAIN}.${STAGINGCC}:"

STAGINGCC = "gcc-cross-initial-${TARGET_ARCH}"
STAGINGCC_class-nativesdk = "gcc-crosssdk-initial-${SDK_SYS}"

do_compile () {
    oe_runmake libbase
    oe_runmake linker
}

do_install () {
    install -d ${D}${base_libdir}
    oe_runmake install-linker
}

FILES_${PN} += "/lib/linker /lib/linker64"