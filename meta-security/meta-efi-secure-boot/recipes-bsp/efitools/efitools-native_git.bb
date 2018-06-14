require efitools.inc

DEPENDS_append = " gnu-efi-native"

inherit native

EXTRA_OEMAKE_append = "\
    INCDIR_PREFIX='${STAGING_DIR_NATIVE}' \
    CRTPATH_PREFIX='${STAGING_DIR_NATIVE}' \
    EXTRA_LDFLAGS='-Wl,-rpath,${libdir}' \
"
