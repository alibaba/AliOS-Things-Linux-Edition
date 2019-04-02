# bionic-linker needs g++ to build, and it only supports the gold linker
EXTRA_OECONF_append_libc-bionic = " --enable-eh-frame-hdr-for-static --enable-gold=default --disable-tls --enable-gnu-indirect-function --enable-initfini-array"
EXTRA_OECONF_append_libc-bionic = " --enable-languages=c,c++"
EXTRA_OECONF_remove_libc-bionic = " --with-ld=${STAGING_BINDIR_TOOLCHAIN}/${TARGET_PREFIX}ld.bfd"